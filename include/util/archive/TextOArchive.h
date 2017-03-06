// TextOArchive.h

#ifndef _UTIL_ARCHIVE_TEXT_O_ARCHIVE_H_
#define _UTIL_ARCHIVE_TEXT_O_ARCHIVE_H_

#include "util/archive/StreamOArchive.h"

#include <ostream>
#include <string>

namespace util
{
    namespace archive
    {

        template <
            typename _Elem = char, 
            typename _Traits = std::char_traits<_Elem>
        >
        class TextOArchive
            : public StreamOArchive<TextOArchive<_Elem, _Traits>, _Elem, _Traits>
        {
        public:
            TextOArchive(
                std::basic_ostream<_Elem, _Traits> & os)
                : StreamOArchive<TextOArchive<_Elem, _Traits>, _Elem, _Traits>(*os.rdbuf())
                , os_(os)
                , local_os_(false)
                , delimiter_(none)
                , space_(" ")
                , newline_("\n")
                , no_string_size_(false)
            {
            }

            TextOArchive(
                std::basic_streambuf<_Elem, _Traits> & buf)
                : StreamOArchive<TextOArchive<_Elem, _Traits>, _Elem, _Traits>(buf)
                , os_(*new std::basic_ostream<_Elem, _Traits>(&buf))
                , local_os_(true)
                , delimiter_(none)
                , space_(" ")
                , newline_("\n")
                , no_string_size_(false)
            {
            }

            ~TextOArchive()
            {
                if (local_os_)
                    delete &os_;
            }

        public:
            /// ������д����������ͱ���
            template<class T>
            void save(
                T const & t)
            {
                newtoken();
                if (this->state())
                    return;
                os_ << t;
                if(os_.fail())
                    this->state(1);
            }

            /// ������д���׼���ַ���
            void save(
                std::string const & t)
            {
                if (!no_string_size_)
                    save((std::size_t)t.size());
                newtoken(); // ����һ���ո�
                this->save_binary((_Elem const *)&t[0], t.size());
            }

            using StreamOArchive<TextOArchive, _Elem, _Traits>::save;

            void save_start(
                std::string const & name)
            {
                if (!delim_.empty()) {
                    newtoken();
                    os_ << name << delim_;
                    delimiter_ = none;
                }
            }

        public:
            /// �Ի��з�Ϊ�ָ�������һ������������һ�������Ȼ���Զ��ָ�Ϊ�ո�ָ���
            void newline()
            {
                delimiter_ = eol;
            }

            void set_delim(
                std::string const & s)
            {
                delim_ = s;
            }

            void set_space(
                std::string const & s)
            {
                space_ = s;
            }

            void set_newline(
                std::string const & s)
            {
                newline_ = s;
            }

            void set_no_string_size(
                bool b)
            {
                no_string_size_ = b;
            }

        private:
            enum  DelimiterType {
                none,
                eol,
                space
            };

        private:
            /// �ָ���һ��ʼ�ǿյģ�none����д���һ�������Ժ��Ϊ�ո�space��
            void newtoken()
            {
                if (this->state())
                    return;
                switch (delimiter_)
                {
                default:
                    this->state(1);
                    break;
                case eol:
                    if (!(os_ << newline_))
                        this->state(1);
                    // �Զ��ָ�Ϊ�ո�ָ���
                    delimiter_ = space;
                    break;
                case space:
                    if (!(os_ << space_))
                        this->state(1);
                    break;
                case none:
                    delimiter_ = space;
                    break;
                }
            }

        private:
            std::basic_ostream<_Elem, _Traits> & os_;
            bool local_os_;
            DelimiterType delimiter_;
            std::string delim_;
            std::string space_;
            std::string newline_;
            bool no_string_size_;
        };

    }  // namespace archive
} // namespace util

#endif // _UTIL_ARCHIVE_TEXT_O_ARCHIVE_H_
