// Pointer.h

#ifndef _FRAMEWORK_GENERIC_POINTER_H_
#define _FRAMEWORK_GENERIC_POINTER_H_

namespace framework
{
    namespace generic
    {

        template <typename _Ty, typename _Pt>
        class Pointer
        {
        public:
            typedef _Ty value_type;

        public:
            _Ty * operator->() const
            {
                return &*This();
            }

            _Ty & operator[](
                size_t idx) const
            {
                return (&*This())[idx];
            }

            _Ty * get() const
            {
                return &*This();
            }

            operator _Ty *() const
            {
                return get();
            }

            void release()
            {
                This() = _Pt();
            }

            Pointer & operator++()
            {
                This() = _Pt(++(&*This()));
                return This();
            }

            friend Pointer const & operator++(_Pt & p, int)
            {
                _Pt tmp(p);
                ++p;
                return tmp;
            }

            Pointer & operator--()
            {
                This() = _Pt(--(&*This()));
                return This();
            }

            friend Pointer const & operator--(_Pt & p, int)
            {
                _Pt tmp(p);
                --p;
                return tmp;
            }

            operator bool() const
            {
                return This() != _Pt();
            }

            bool operator!() const
            {
                return !(bool)This();
            }

            friend bool operator!= (
                Pointer const & l, 
                Pointer const & r)
            {
                return !(static_cast<_Pt const &>(l) == static_cast<_Pt const &>(r));
            }

        protected:
            _Pt & This()
            {
                return static_cast<_Pt &>(*this);
            }

            _Pt const & This() const
            {
                return static_cast<_Pt const &>(*this);
            }

        };

    } // namespace generic
} // namespace framework

#endif // _FRAMEWORK_GENERIC_POINTER_H_
