#ifndef _DEQUEEX_H
#define _DEQUEEX_H

#include <vector>

namespace std {
    template<typename T>
    class DequeEx :
        public vector<T>
    {
    public:
        /*
        typedef typename list<T>::iterator iterator;
        typedef typename list<T>::const_iterator const_iterator;

        T& operator[](size_t pos)
        {
        if (list<T>::size() <= pos) {
        throw runtime_error("dequeex subscript out of range");
        }
        iterator iter = list<T>::begin();
        for (size_t i=0; i<pos; i++)
        iter++;
        return (*iter);
        }

        T const& operator[](size_t pos) const
        {
        if (list<T>::size() <= pos) {
        throw runtime_error("dequeex subscript out of range");
        }
        const_iterator iter = list<T>::begin();
        for (size_t i=0; i<pos; i++)
        iter++;
        return (*iter);
        }
        */
        void pop_front()
        {
            if (vector<T>::size()==0)
                return;
            vector<T>::erase(vector<T>::begin());
        }

        void push_front(const T &val)
        {
            vector<T>::insert(vector<T>::begin(), val);
        }
    };
}
#endif
