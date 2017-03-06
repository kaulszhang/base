#ifndef _FRAMEWORK_CONTAINER_ITERATOR_H_
#define _FRAMEWORK_CONTAINER_ITERATOR_H_

namespace framework
{
    namespace container
    {

        template <typename _Ct>
        typename _Ct::iterator begin(
            _Ct & ct)
        {
            return ct.begin();
        }

        template <typename _Ct>
        typename _Ct::const_iterator cbegin(
            _Ct const & ct)
        {
            return ct.begin();
        }

        template <typename _Ct>
        typename _Ct::iterator end(
            _Ct & ct)
        {
            return ct.end();
        }

        template <typename _Ct>
        typename _Ct::const_iterator cend(
            _Ct const & ct)
        {
            return ct.end();
        }

    } // namespace container
} // namespace framework

#endif // #ifndef _FRAMEWORK_CONTAINER_ITERATOR_H_
