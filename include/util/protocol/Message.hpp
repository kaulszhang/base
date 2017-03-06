// Message.hpp

#ifndef _UTIL_PROTOCOL_MESSAGE_HPP_
#define _UTIL_PROTOCOL_MESSAGE_HPP_

#include "util/protocol/Message.h"
#include "util/protocol/MessageTraits.h"

namespace util
{
    namespace protocol
    {

        template <
            typename MsgT
        >
        std::map<typename Message<MsgT>::id_type, MessageDefine const *> & Message<MsgT>::msg_defs()
        {
            static std::map<typename Message<MsgT>::id_type, MessageDefine const *> smap;
            return smap;
        }

        template <typename MsgT>
        Message<MsgT>::Message()
        {
        }

        template <typename MsgT>
        Message<MsgT>::Message(
            Message const & r)
            : MessageBase(r)
            , header_type(r)
        {
            // default copy constructor will copy member "data_", which is not expected to 
        }

        template <typename MsgT>
        template <typename T>
        T & Message<MsgT>::get()
        {
            if (!is<T>()) {
                MessageBase::reset(&T::msg_def);
                header_type::id(T::static_id);
            }
            return as<T>();
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::reset(
            T const & t)
        {
            reset();
            header_type::id(T::static_id);
            def_ = &T::msg_def;
            assert(sizeof(T) <= sizeof(data_));
            new (data_) T(t);
        }

        template <typename MsgT>
        template <typename T>
        bool Message<MsgT>::is() const
        {
            return def_ == &T::msg_def;
        }

        template <typename MsgT>
        template <typename T>
        T const & Message<MsgT>::as() const
        {
            assert(is<T>());
            return *(T const *)data_;
        }

        template <typename MsgT>
        template <typename T>
        T & Message<MsgT>::as()
        {
            assert(is<T>());
            return *(T *)data_;
        }

        template <typename MsgT>
        void Message<MsgT>::reset()
        {
            MessageBase::reset();
            (header_type &)(*this) = header_type();
        }

        template <typename MsgT>
        void Message<MsgT>::from_data(
            StreamBuffer & buf, 
            void * vctx)
        {
            typename MsgT::i_archive_t ia(buf);
            typename MsgT::context_t * ctx = 
                reinterpret_cast<typename MsgT::context_t *>(vctx);
            typename MsgT::helper_t hlp(ia, *this, ctx);
            ia >> (header_type &)(*this);
            MessageBase::reset(find_msg(header_type::id()));
            MessageBase::from_data(&ia, &hlp);
        }

        template <typename MsgT>
        void Message<MsgT>::to_data(
            StreamBuffer & buf, 
            void * vctx) const
        {
            typename MsgT::o_archive_t oa(buf);
            typename MsgT::context_t * ctx = 
                reinterpret_cast<typename MsgT::context_t *>(vctx);
            typename MsgT::helper_t hlp(oa, *this, ctx);
            oa << (header_type const &)(*this);
            MessageBase::to_data(&oa, &hlp);
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::reg_msg(
            MessageDefine & def)
        {
            assert(&def == &T::msg_def);
            def.construct = &s_construct<T>;
            def.copy = &s_copy<T>;
            def.from_data = &s_from_data<T>;
            def.to_data = &s_to_data<T>;
            def.destroy = &s_destroy<T>;
            msg_defs()[T::static_id] = &def;
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_construct(
            MessageBase * mb)
        {
            Message * m = static_cast<Message *>(mb);
            assert(sizeof(T) <= sizeof(m->data_));
            new (m->data_) T;
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_copy(
            MessageBase * mb, 
            MessageBase const * mbr)
        {
            Message * m = static_cast<Message *>(mb);
            Message const * mr = static_cast<Message const *>(mbr);
            new (m->data_) T(mr->as<T>());
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_from_data(
            MessageBase * mb, 
            void * ar, 
            void * vctx)
        {
            Message * m = static_cast<Message *>(mb);
            typename MsgT::i_archive_t * ia = 
                reinterpret_cast<typename MsgT::i_archive_t *>(ar);
            typename MsgT::helper_t * hlp = 
                reinterpret_cast<typename MsgT::helper_t *>(vctx);
            hlp->begin_data();
            (*ia) >> m->as<T>();
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_to_data(
            MessageBase const * mb, 
            void * ar, 
            void * vctx)
        {
            Message const * m = static_cast<Message const *>(mb);
            typename MsgT::o_archive_t * oa = 
                reinterpret_cast<typename MsgT::o_archive_t *>(ar);
            typename MsgT::helper_t * hlp = 
                reinterpret_cast<typename MsgT::helper_t *>(vctx);
            hlp->begin_data();
            (*oa) << m->as<T>();
        }

        template <typename MsgT>
        template <typename T>
        void Message<MsgT>::s_destroy(
            MessageBase * mb)
        {
            Message * m = static_cast<Message *>(mb);
            ((T *)m->data_)->~T();
        }

    } // namespace protocol
} // namespace util

#endif // _UTIL_PROTOCOL_MESSAGE_HPP_
