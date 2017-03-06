// MessageQueue.h

#ifndef _FRAMEWORK_PROCESS_MESSAGE_QUEUE_H_
#define _FRAMEWORK_PROCESS_MESSAGE_QUEUE_H_

#define MESSAGE_QUEUE_SHARED_MEMORY_INST_ID 1

namespace framework
{
    namespace memory
    {
        class SharedMemory;
    }

    namespace process
    {

        struct Message
        {
            Message & operator=(
                Message const & r)
            {
                sender = r.sender;
                receiver = r.receiver;
                level    = r.level;
                type     = r.type;
                data     = r.data;
                time     = r.time;
                return *this;
            }

            std::string     sender;
            std::string     receiver;
            boost::uint32_t level;
            boost::uint32_t type;
            std::string     data;
            boost::uint32_t time;
        };

        class MessageQueue
        {
        public:
            MessageQueue(
                std::string const & owner, 
                framework::memory::SharedMemory & shm);

            ~MessageQueue();

            void pop(
                std::vector<Message> & msg_list, 
                Message const & hint, 
                boost::uint32_t out_size);

            bool pop(
                Message & msg);

            // push 如果没有指定receiver就发给所有人包括自己
            void push_all(
                Message const & msg);

            // push 如果没有指定receiver就发给所有人,不包括自己
            boost::uint32_t push(
                Message const & msg);

            void users(
                std::vector<std::string> & list);

            void clear(void);

        private:
            struct InternalMessage;
            struct User;

            User * add_and_get_user(
                std::string const & user);

            User * get_user_by_id(
                boost::uint32_t id);

            User * find_user(
                std::string const & user);

            User * find_user_from_cache(
                std::string const & user);

            bool release_message(void);

            void release_and_get_message_data(
                InternalMessage * imsg, 
                std::string & data);

            void release_message_data(
                InternalMessage * imsg);

            void alloc_message_space(
                char * buf, 
                boost::uint32_t page_count);

            template <typename T>
            T * alloc_object()
            {
                void * addr = alloc();
                return new (addr) T;
            }

            template <typename T>
            void free_object(T * t)
            {
                t->~T();
                free(t);
            }

            void * alloc();

            void free(
                void * t);

            bool push_message_to_user(
                Message const & msg, 
                std::string const & user);

            void insert_child_node(
                InternalMessage * imsg, 
                std::string const & data);

            boost::uint32_t message_childnode_size(
                InternalMessage const * imsg);

            void print_size(void);

        private:
            struct Entry;

            std::string owner_; 
            framework::memory::SharedMemory & shm_;
            Entry * entry_;
            std::map<std::string, User *> user_cache_;
            boost::uint32_t user_id_;
        };

    } // namespace process
} // namespace framework

#endif // _FRAMEWORK_PROCESS_MESSAGE_QUEUE_H_
