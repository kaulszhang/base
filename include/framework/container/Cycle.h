// CycleBuffer.h

#ifndef _FRAMEWORK_CONTAINER_CYCLE_BUFFER_H_
#define _FRAMEWORK_CONTAINER_CYCLE_BUFFER_H_

#include <boost/operators.hpp>

namespace framework
{
    namespace container
    {

        template<
            typename _Ty
        >
        class cycle_const_iterator;

        template<
            typename _Ty
        >
        class cycle_iterator
            : public boost::forward_iterator_helper<
                cycle_iterator<_Ty>, 
                _Ty
            >
        {
        public:
            cycle_iterator(
                _Ty * beg, 
                _Ty * cur, 
                _Ty * end)
                : beg_(beg)
                , cur_(cur)
                , end_(end)
            {
            }

            cycle_iterator(
                cycle_iterator const & r)
                : beg_(r.beg_)
                , cur_(r.cur_)
                , end_(r.end_)
            {
            }

        public:
            _Ty & operator*() const
            {
                return *cur_;
            }

            cycle_iterator & operator++()
            {
                ++cur_;
                if (cur_ == end_)
                    cur_ = beg_;
                return *this;
            }

        public:
            friend bool operator ==(
                cycle_iterator const & l,
                cycle_iterator const & r)
            {
                assert(l.beg_ == r.beg_ && l.end_ == r.end_);
                return l.cur_ == r.cur_;
            }

        private:
            friend class cycle_const_iterator<_Ty>;

        private:
            _Ty * beg_;
            _Ty * cur_;
            _Ty * end_;
        };

        template<
            typename _Ty
        >
        class cycle_const_iterator
            : public boost::forward_iterator_helper<
                cycle_const_iterator<_Ty>, 
                _Ty const
            >
        {
        public:
            cycle_const_iterator(
                _Ty const * beg, 
                _Ty const * cur, 
                _Ty const * end)
                : beg_(beg)
                , cur_(cur)
                , end_(end)
            {
            }

            cycle_const_iterator(
                cycle_const_iterator const & r)
                : beg_(r.beg_)
                , cur_(r.cur_)
                , end_(r.end_)
            {
            }

            cycle_const_iterator(
                cycle_iterator<_Ty> const & r)
                : beg_(r.beg_)
                , cur_(r.cur_)
                , end_(r.end_)
            {
            }

        public:
            _Ty const & operator*() const
            {
                return *cur_;
            }

            cycle_const_iterator & operator++()
            {
                ++cur_;
                if (cur_ == end_)
                    cur_ = beg_;
                return *this;
            }

        public:
            friend bool operator ==(
                cycle_const_iterator const & l,
                cycle_const_iterator const & r)
            {
                assert(l.beg_ == r.beg_ && l.end_ == r.end_);
                return l.cur_ == r.cur_;
            }

        private:
            _Ty const * beg_;
            _Ty const * cur_;
            _Ty const * end_;
        };

        template <
            typename _Ty
        >
        class Cycle
        {
        public:
            typedef _Ty value_type;

            typedef cycle_iterator<value_type> iterator;

            typedef cycle_const_iterator<value_type> const_iterator;

        public:
            Cycle(
                size_t capacity)
                : m_capacity(capacity)
                , m_datas(new value_type[capacity])
                , m_write_index(0)
                , m_read_index(0)
                , m_size(0)
            {

            }

            ~Cycle()
            {
                if (m_datas)
                    delete [] m_datas;
            }

        public:
            void push(
                value_type const & t)
            {
                assert(m_size < m_capacity);
                m_datas[m_write_index] = t;
                ++m_write_index;
                ++m_size;
            }

            void pop()
            {
                assert(m_size > 0);
                ++ m_read_index;
                --m_size;
                if (m_read_index == m_capacity) {
                    m_read_index = 0;
                }
            }

        public:
            size_t capacity() const
            {
                return m_capacity;
            }

            size_t size() const
            {
                return m_size;
            }

            value_type const & front() const
            {
                assert(m_size > 0);
                return m_datas[m_read_index];
            }

            value_type & front()
            {
                assert(m_size > 0);
                return m_datas[m_read_index];
            }

        public:
            iterator begin()
            {
                return iterator(m_datas, m_datas + m_read_index, m_datas + m_capacity);
            }

            iterator end()
            {
                return iterator(m_datas, m_datas + m_write_index, m_datas + m_capacity);
            }

            const_iterator begin() const
            {
                return const_iterator(m_datas, m_datas + m_read_index, m_datas + m_capacity);
            }

            const_iterator end() const
            {
                return const_iterator(m_datas, m_datas + m_write_index, m_datas + m_capacity);
            }

        private:
            size_t m_capacity;

            value_type * m_datas;

            volatile size_t m_write_index;

            volatile size_t m_read_index;

            volatile size_t m_size;
        };

    } // namespace container
} // namespace framework

#endif // _FRAMEWORK_CONTAINER_CYCLE_BUFFER_H_
