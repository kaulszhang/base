// ResolverIterator.h

#ifndef _FRAMEWORK_NETWORK_RESOLVER_ITERATOR_H_
#define _FRAMEWORK_NETWORK_RESOLVER_ITERATOR_H_

#include <framework/network/Endpoint.h>

#include <boost/operators.hpp>

namespace framework
{
    namespace network
    {

        class ResolverServiceImpl;

        class ResolverIterator
            : public boost::forward_iterator_helper<ResolverIterator, Endpoint const>
        {
        public:
            typedef Endpoint const value_type;

            typedef ResolverService::implementation_type implementation_type;

            ResolverIterator()
                : service_(NULL)
                , current_(size_t(-1))
            {
            }

            ResolverIterator(
                ResolverService & service, 
                implementation_type impl, 
                Endpoint const & endpoint)
                : service_(&service)
                , impl_(impl)
                , current_(1)
                , endpoint_(endpoint)
            {
            }

            ~ResolverIterator()
            {
            }

            ResolverIterator & operator++()            {                service_->increment(*this);                return *this;            }

            value_type & operator*() const
            {
                return endpoint_;
            }

            friend bool operator==(
                ResolverIterator const & l, 
                ResolverIterator const & r)
            {
                return l.current_ == r.current_;
            }

        private:
            friend class ResolverServiceImpl;
            ResolverService * service_;
            implementation_type impl_;
            size_t current_;
            Endpoint endpoint_;
        };

    }
}

#endif // _FRAMEWORK_NETWORK_RESOLVER_ITERATOR_H_
