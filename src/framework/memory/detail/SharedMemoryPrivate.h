// SharedMemoryPrivate.h

#ifndef _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_PRIVATE_H_
#define _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_PRIVATE_H_

#include "framework/memory/detail/SharedMemoryImpl.h"
#include "framework/memory/PrivateMemory.h"

namespace framework
{
    namespace memory
    {

        namespace detail
        {

            class SharedMemoryPrivate
                : public SharedMemoryImpl
            {
            private:
                static boost::system::error_code make_error_code(
                    boost::system::posix_error::errc_t e)
                {
                    return boost::system::error_code(e, boost::system::get_generic_category());
                }

                bool create( 
                    void ** id, 
                    boost::uint32_t iid, 
                    boost::uint32_t key, 
                    boost::uint32_t size, 
                    boost::system::error_code & ec)
                {
                    BlockSetSet::iterator iter = 
                        blocks_.insert(std::make_pair(iid, BlockSet())).first;

                    std::pair<BlockSet::iterator, bool> iter2 = 
                        iter->second.insert(std::make_pair(key, std::make_pair((void *)0, 0)));

                    if (!iter2.second) {
                        ec = make_error_code(boost::system::posix_error::file_exists);
                        return false;
                    }

                    void * p = memory_.alloc_block(size);

                    if (p == NULL) {
                        iter->second.erase(iter2.first);
                        ec = make_error_code(boost::system::posix_error::not_enough_memory);
                        return false;
                    }

                    iter2.first->second.first = p;
                    iter2.first->second.second = 1;
                    *id = iter2.first->second.first;

                    blocks2_.insert(std::make_pair(*id, std::make_pair(iid, key)));

                    return true;
                }

                bool open( 
                    void ** id, 
                    boost::uint32_t iid,
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
                    BlockSetSet::iterator iter = 
                        blocks_.find(iid);

                    if (iter == blocks_.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    BlockSet::iterator iter2 = 
                        iter->second.find(key);

                    if (iter2 == iter->second.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    iter2->second.second++;
                    *id = iter2->second.first;

                    return true;
                }

                void * map(
                    void * id,
                    boost::uint32_t size,
                    boost::system::error_code & ec )
                {
                    return id;
                }

                bool unmap(
                    void * addr, 
                    boost::uint32_t size,
                    boost::system::error_code & ec)
                {
                    return true;
                }

                bool close(
                    void * id, 
                    boost::system::error_code & ec)
                {
                    BlockSet2::iterator iter3 = 
                        blocks2_.find(id);

                    assert(iter3 != blocks2_.end());
                    if (iter3 == blocks2_.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    BlockSetSet::iterator iter = 
                        blocks_.find(iter3->second.first);

                    assert(iter != blocks_.end());
                    if (iter == blocks_.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    BlockSet::iterator iter2 = 
                        iter->second.find(iter3->second.second);

                    assert(iter2 != iter->second.end());
                    if (iter2 == iter->second.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    assert(iter2->second.second > 0);
                    assert(id == iter2->second.first);
                    iter2->second.second--;

                    return true;
                }

                bool destory( 
                    boost::uint32_t iid, 
                    boost::uint32_t key,
                    boost::system::error_code & ec)
                {
                    BlockSetSet::iterator iter = 
                        blocks_.find(iid);

                    assert(iter != blocks_.end());
                    if (iter == blocks_.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    BlockSet::iterator iter2 = 
                        iter->second.find(key);

                    assert(iter2 != iter->second.end());
                    if (iter2 == iter->second.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    if (iter2->second.second != 0) {
                        return false;
                    }

                    BlockSet2::iterator iter3 = 
                        blocks2_.find(iter2->second.first);

                    assert(iter3 != blocks2_.end());
                    if (iter3 == blocks2_.end()) {
                        ec = make_error_code(boost::system::posix_error::no_such_file_or_directory);
                        return false;
                    }

                    blocks2_.erase(iter3);
                    iter->second.erase(iter2);
                    if (iter->second.empty()) {
                        blocks_.erase(iter);
                    }

                    return true;
                }

            private:
                PrivateMemory memory_;
                typedef std::pair<void *, size_t> Block; // id, nref
                typedef std::map<boost::uint32_t, Block> BlockSet; // key -> Block
                typedef std::map<boost::uint32_t, BlockSet> BlockSetSet; // iid -> BlockSet
                typedef std::pair<boost::uint32_t, boost::uint32_t> Block2; // iid, key
                typedef std::map<void *, Block2> BlockSet2; // id - > Block2
                BlockSetSet blocks_;
                BlockSet2 blocks2_;
            };

            static SharedMemoryPrivate shared_memory_private;

        } // namespace detail

    } // namespace memory
} // namespace framework

#endif // _FRAMEWORK_MEMORY_DETAIL_SHARED_MEMORY_PRIVATE_H_
