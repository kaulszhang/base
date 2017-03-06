// MessageQueue.cpp

#include "framework/Framework.h"
#include "framework/process/MessageQueue.h"
#include "framework/container/List.h"
#include "framework/memory/MemoryPage.h"
#include "framework/memory/SharedMemory.h"
#include "framework/memory/SharedMemoryPointer.h"
#include "framework/memory/MemoryReference.h"
#include "framework/memory/SharedMemoryIdPointer.h"
#include "framework/process/Mutex.h"

using namespace framework::memory;

#define     QUEUE_OBJECT_ID               101
#define     MESSAGE_BLOCK_SIZE            256

namespace framework
{
    namespace process
    {

        typedef std::vector<Message> MessageList;
        typedef std::vector<std::string> UserList;

        struct MessageNode
        {
            framework::memory::SharedMemoryPointer<MessageNode, MESSAGE_QUEUE_SHARED_MEMORY_INST_ID> next;
            boost::uint32_t size;
            boost::uint8_t  data[1];
        };

        struct MessageQueue::InternalMessage
        {
            struct Hook
                : framework::container::ListHook<
                Hook,
                framework::memory::SharedMemoryPointerTraits<Hook, MESSAGE_QUEUE_SHARED_MEMORY_INST_ID>
                >::type
            {
            };

            typedef framework::container::List<Hook> ListType;

            Hook            hook[2];
            boost::uint32_t level;
            boost::uint32_t type;
            boost::uint32_t receiver_id;
            boost::uint32_t sender_id;
            boost::uint32_t size;
            boost::uint32_t time;
            framework::memory::SharedMemoryPointer<MessageNode, MESSAGE_QUEUE_SHARED_MEMORY_INST_ID> next;
            boost::uint8_t  data[1];
        };

        struct MemoryObject
            : framework::container::ListHook<
            MemoryObject
            , framework::memory::SharedMemoryPointerTraits<MemoryObject, MESSAGE_QUEUE_SHARED_MEMORY_INST_ID>
            >::type
        {
        };

        typedef framework::container::List<MemoryObject> MemoryObjectPool;

        struct MessageQueue::User
        {
            struct Hook
                : framework::container::ListHook<
                Hook
                , framework::memory::SharedMemoryPointerTraits<Hook, MESSAGE_QUEUE_SHARED_MEMORY_INST_ID>
                >::type
            {
            };

            typedef framework::container::List<Hook> ListType;

            Hook                             hook;
            boost::uint32_t                  id;
            char                             name[32];
            boost::uint32_t                  name_size;
            InternalMessage::ListType        messages;
        };

        struct MessageQueue::Entry
        {
            Entry ()
                : is_alloc(0)
                , is_first_alloc(0)
            {
            }

            static const size_t msg_head_size = sizeof(InternalMessage);
            static const size_t msg_data_size = MESSAGE_BLOCK_SIZE - msg_head_size;
            static const size_t msgnode_head_size = sizeof(MessageNode);
            static const size_t msgnode_data_size = MESSAGE_BLOCK_SIZE - msgnode_head_size;

            User::ListType            users;
            InternalMessage::ListType level_messages[8];
            MemoryObjectPool          free_messages_;
            framework::process::Mutex mutex;
            boost::uint32_t           is_alloc;
            boost::uint32_t           is_first_alloc;
        };

        MessageQueue::MessageQueue(
            std::string const & owner, 
            SharedMemory & shm)
            : owner_(owner)
            , shm_(shm)
            , entry_(NULL)
        {
            entry_ = (Entry *)shm_.alloc_with_id(QUEUE_OBJECT_ID, sizeof(Entry));
            if (entry_) {
                new (entry_) Entry;
            } else {
                entry_ = (Entry *)shm_.get_by_id(QUEUE_OBJECT_ID);
            }

            entry_->mutex.lock();
            if (!entry_->is_alloc) {
                size_t message_total_size = 1 * framework::memory::MemoryPage::page_size();
                char * buffer = (char *)shm_.alloc_block(message_total_size);
                alloc_message_space(buffer, 1);
                entry_->is_alloc = 1;
            }
            user_id_ = add_and_get_user(owner_)->id;
            entry_->mutex.unlock();
            print_size();
        }

        MessageQueue::~MessageQueue()
        {
            if (entry_) {
                //delete entry_;
                entry_ = NULL;
            }
        }

        void MessageQueue::pop(
            std::vector<Message> & msg_list, 
            Message const & hint, 
            boost::uint32_t out_size)
        {
            boost::uint32_t size = 0;
            while(true) {
                Message   msg = hint;
                if (pop(msg)) {
                    msg_list.push_back(msg);
                    ++size;
                    if (size >= out_size) {
                        break;
                    }
                } else {
                    break;
                }
            }
            print_size();
        }

        bool MessageQueue::pop(
            Message & msg)
        {
            bool res = false;
            std::string sender = msg.sender;
            std::string receiver;
            if (msg.receiver.empty()) {
                receiver = owner_;
            } else {
                receiver = msg.receiver;
            }
            entry_->mutex.lock();
            User * p_user = find_user(receiver);
            User * p_sender = NULL;
            if (!sender.empty()) {
                p_sender = find_user(sender);
                if (!p_sender) {
                    entry_->mutex.unlock();
                    return res;
                }
            }

            if (p_user) {
                for (InternalMessage::ListType::iterator iter = p_user->messages.begin();
                    iter != p_user->messages.end();
                    iter++)
                {
                    InternalMessage * imsg = (InternalMessage *)&(*iter);
                    if (imsg->level <= msg.level && msg.type == imsg->type) {
                        if (p_sender) {
                            if( p_sender->id != imsg->sender_id ) {
                                continue;
                            }
                        }

                        msg.level = imsg->level;
                        msg.type  = imsg->type;
                        msg.time  = imsg->time;
                        if (sender.empty()) {
                            User *tmp = get_user_by_id(imsg->sender_id);
                            msg.sender.assign((char*)tmp->name, tmp->name_size);
                        } else {
                            msg.sender = sender;
                        }
                        msg.receiver = receiver;
                        // 从用户收信箱删除
                        p_user->messages.erase(iter);
                        // 从同level信箱中删除
                        entry_->level_messages[msg.level].erase(imsg->hook+1);
                        // 回收信息空间
                        std::string data;
                        release_and_get_message_data(imsg, data);
                        msg.data = data;
                        res = true;
                        break;
                    }
                }
            }
            print_size();
            entry_->mutex.unlock();
            return res;
        }

        // push 如果没有指定receiver就发给所有人包括自己
        void MessageQueue::push_all(
            Message const & msg)
        {
            entry_->mutex.lock();
            if (!msg.receiver.empty()) {
                push_message_to_user(msg, msg.receiver);
            } else {
                std::string user_name;
                User * p_user = NULL;
                for(User::ListType::iterator iter = entry_->users.begin();
                    iter != entry_->users.end();
                    iter++)
                {
                    p_user = (User *)&(*iter);
                    user_name.assign((char*)p_user->name, p_user->name_size);
                    push_message_to_user(msg, user_name);
                }
            }
            print_size();
            entry_->mutex.unlock();
        }

        // push 如果没有指定receiver就发给所有人,不包括自己
        boost::uint32_t MessageQueue::push(
            Message const & msg)
        {
            boost::uint32_t sum = 0;
            entry_->mutex.lock();
            if (!msg.receiver.empty()) {
                if (push_message_to_user(msg, msg.receiver)) {
                    ++sum;
                }
            } else {
                std::string user_name;
                User * p_user = NULL;
                for(User::ListType::iterator iter = entry_->users.begin();
                    iter != entry_->users.end();
                    iter++)
                {
                    p_user = (User *)&(*iter);
                    user_name.assign((char*)p_user->name, p_user->name_size);
                    if (user_name != owner_) {
                        if (push_message_to_user(msg, user_name)) {
                            ++sum;
                        }
                    }
                }
            }
            print_size();
            entry_->mutex.unlock();
            return sum;
        }

        void MessageQueue::users(
            std::vector<std::string> & list)
        {
            std::string user_name;
            User * p_user = NULL;
            entry_->mutex.lock();
            for(User::ListType::iterator iter = entry_->users.begin();
                iter != entry_->users.end();
                iter++)
            {
                p_user = (User *)&(*iter);
                user_name.clear();
                user_name.assign((char*)p_user->name, p_user->name_size);
                list.push_back(user_name);
            }
            entry_->mutex.unlock();
        }

        void MessageQueue::clear(void)
        {
            entry_->mutex.lock();
            User * p_user = find_user(owner_);
            InternalMessage * imsg = NULL;
            if (p_user) {
                for (InternalMessage::ListType::iterator iter = p_user->messages.begin();
                    iter != p_user->messages.end();) {
                        imsg = (InternalMessage *)&(*iter);
                        entry_->level_messages[imsg->level].erase(imsg->hook + 1);
                        iter = p_user->messages.erase(iter);
                        release_message_data(imsg);
                }
            }
            print_size();
            entry_->mutex.unlock();
        }

        MessageQueue::User * MessageQueue::add_and_get_user(
            std::string const & user)
        {
            User * p_user = find_user(user);
            if (!p_user) {
                p_user = alloc_object<User>();
                if (user.size() <= 32) {
                    memcpy(p_user->name, user.c_str(), user.size());
                    p_user->name_size = user.size();
                } else {
                    memcpy(p_user->name, user.c_str(), 32);
                    p_user->name_size = 32;
                }
                p_user->id = entry_->users.size() + 1;
                entry_->users.insert(&p_user->hook);
                user_cache_.insert(std::pair<std::string , User *>(user, p_user));
            }
            return p_user;
        }

        MessageQueue::User * MessageQueue::get_user_by_id(
            boost::uint32_t id)
        {
            User * p_user = NULL;
            bool res = false;
            for(User::ListType::iterator iter = entry_->users.begin();
                iter != entry_->users.end();
                iter++)
            {
                p_user = (User *)&(*iter);
                if (p_user->id == id) {
                    res = true;
                    break;
                }
            }
            if (!res) {
                p_user = NULL;
            }
            return p_user;
        }

        MessageQueue::User * MessageQueue::find_user(
            std::string const & user)
        {
            User * p_user = find_user_from_cache(user);
            if (!p_user) {
                bool res = false;
                for(User::ListType::iterator iter = entry_->users.begin();
                    iter != entry_->users.end();
                    iter++)
                {
                    p_user = (User *)&(*iter);
                    if (user.size() <= 32) {
                        if(memcmp(p_user->name, user.c_str(), user.size()) == 0) {
                            res = true;
                            break;
                        }
                    } else {
                        if(memcmp(p_user->name, user.c_str(), 32)) {
                            res = true;
                            break;
                        }
                    }
                }
                if (!res) {
                    p_user = NULL;
                } else {
                    std::map<std::string, User *>::iterator iter;
                    iter = user_cache_.find(user);
                    if (iter == user_cache_.end()) {
                        user_cache_.insert(std::pair<std::string , User *>(user, p_user));
                    }
                }
            }
            return p_user;
        }

        MessageQueue::User * MessageQueue::find_user_from_cache(
            std::string const & user)
        {
            User * p_user = NULL;
            std::map<std::string, User *>::iterator iter;
            iter = user_cache_.find(user);
            if (iter != user_cache_.end()) {
                p_user = (*iter).second;
            }
            return p_user;
        }

        bool MessageQueue::release_message(void)
        {
            bool res = false;
            InternalMessage * imsg = NULL;
            for (boost::int32_t i = 7; i >= 0;  i--) {
                for (InternalMessage::ListType::iterator iter = entry_->level_messages[i].begin();
                    iter != entry_->level_messages[i].end();
                    iter++)
                {
                    imsg = (InternalMessage *)(&(*iter) - 1);
                    entry_->level_messages[i].erase(iter);
                    break;
                } // end for

                if (imsg) {
                    User * p_user = get_user_by_id(imsg->receiver_id);
                    if (p_user) {
                        p_user->messages.erase(imsg->hook);
                        std::string data;
                        release_and_get_message_data(imsg, data);
                        res = true;
                        break;
                    }
                }
            }
            return res;
        }

        void MessageQueue::release_and_get_message_data(
            InternalMessage * imsg, 
            std::string & data)
        {
            assert(imsg != NULL);
            std::string node_data;
            node_data.assign((char*)imsg->data, imsg->size);
            data += node_data;
            MessageNode * node = imsg->next;
            MessageNode * delete_node = NULL;
            while (node) {
                node_data.assign((char*)node->data, node->size);
                data += node_data;
                delete_node = node;
                node = node->next;
                free_object<MessageNode>(delete_node);
            }
            free_object<InternalMessage>(imsg);
        }

        void MessageQueue::release_message_data(
            InternalMessage * imsg)
        {
            assert(imsg != NULL);
            MessageNode * node = imsg->next;
            MessageNode * delete_node = NULL;
            while (node) {
                delete_node = node;
                node = node->next;
                free_object<MessageNode>(delete_node);
            }
            free_object<InternalMessage>(imsg);
        }

        void MessageQueue::alloc_message_space(
            char * buf, 
            boost::uint32_t page_count)
        {
            MemoryObject * mem_obj = NULL;
            boost::uint32_t message_count = (page_count * framework::memory::MemoryPage::page_size()) / MESSAGE_BLOCK_SIZE;
            for(boost::uint32_t i = 0; i < message_count; ++i) {
                mem_obj = new ((void*) (buf + i*MESSAGE_BLOCK_SIZE)) MemoryObject;
                entry_->free_messages_.insert(mem_obj);
            }
        }

        void * MessageQueue::alloc()
        {
            if (entry_->free_messages_.empty()) {
                if (!entry_->is_first_alloc) {
                    entry_->is_first_alloc = 1;
                    size_t message_total_size = 100 * framework::memory::MemoryPage::page_size();
                    char * buffer = (char *)shm_.alloc_block(message_total_size);
                    alloc_message_space(buffer, 100);
                } else {
                    if (!release_message()) {
                        std::cout << "release message list failed" << std::endl;
                    }
                }
            }

            void * addr = (void *)&*entry_->free_messages_.first();
            entry_->free_messages_.pop_front();
            return addr;
        }

        void MessageQueue::free(
            void * t)
        {
            MemoryObject * mem_obj = new (t) MemoryObject;
            entry_->free_messages_.insert(mem_obj);
        }

        bool MessageQueue::push_message_to_user(
            Message const & msg, 
            std::string const & user)
        {
            bool res = false;
            if (msg.level >= 8) {
                std::cout << "Level over flow" << std::endl;
                return res;
            }

            User * receiver = find_user(user);
            //User * receiver = add_and_get_user(user);
            if (receiver) {
                InternalMessage * imsg = alloc_object<InternalMessage>();
                imsg->level = msg.level;
                imsg->type = msg.type;
                imsg->receiver_id = receiver->id;
                imsg->sender_id   = user_id_;
                imsg->time        = time(NULL);
                imsg->next        = NULL;
                insert_child_node(imsg, msg.data);
                receiver->messages.insert(imsg->hook);
                entry_->level_messages[msg.level].insert(imsg->hook+1);
                res = true;
            }
            return res;
        }

        void MessageQueue::insert_child_node(
            InternalMessage * imsg, 
            std::string const & data)
        {
            boost::uint32_t data_size = data.size();
            if (data.size() <= entry_->msg_data_size) {
                imsg->size = data_size;
                memcpy(imsg->data, data.c_str(), imsg->size);
            } else {
                boost::uint32_t position = 0;
                imsg->size = entry_->msg_data_size;
                memcpy(imsg->data, data.c_str(), imsg->size);
                position += imsg->size;

                MessageNode * prev = NULL;
                MessageNode * cur = NULL;
                MessageNode * head = NULL;
                while (true) {
                    cur = alloc_object<MessageNode>();
                    cur->next = NULL;
                    if ((data_size-position) <= entry_->msgnode_data_size) {
                        cur->size = data_size-position;
                        memcpy(cur->data, data.c_str()+position, cur->size);
                        position += cur->size;
                        if (!head) {
                            head = cur;
                        } else {
                            prev->next = cur;
                        }
                        break;
                    } else {
                        cur->size = entry_->msgnode_data_size;
                        memcpy(cur->data, data.c_str()+position, cur->size);
                        position += cur->size;
                        if (!head) {
                            head = cur;
                            prev = cur;
                        } else {
                            prev->next = cur;
                            prev = cur;
                        }
                    }
                } // end while
                imsg->next = head;
            }
        }

        boost::uint32_t MessageQueue::message_childnode_size(
            InternalMessage const * imsg)
        {
            assert(imsg != NULL);
            boost::uint32_t count = 0;
            MessageNode * node = imsg->next;
            while (node) {
                ++count;
                node = node->next;
            }
            return count;
        }

        void MessageQueue::print_size(void)
        {
            //std::cout << "============= block begin =================" << std::endl;
            //std::cout << "free message size: " << entry_->free_messages_.size() << std::endl;
            //std::cout << "total user count: " << entry_->users.size() << std::endl;
            //User * p_user = NULL;
            //for(User::ListType::iterator iter = entry_->users.begin();
            //    iter != entry_->users.end();
            //    iter++)
            //{
            //    p_user = (User *)&(*iter);
            //    std::cout << "user id = " << p_user->id << std::endl;
            //    std::cout << "user message numbers = " << p_user->messages.size() << std::endl;
            //}
            //std::cout << "============= block end  =================" << std::endl;
        }

    } // namespace process
} // namespace framework
