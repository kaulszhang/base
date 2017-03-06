// Profile.h

/** һ�������ļ���д��
�����ļ����ݲ���һ�¸�ʽ��
[ģ��1]
���ò���1=����ֵ
���ò���2=����ֵ
[ģ��2]
���ò���1=����ֵ
*/

#ifndef _FRAMEWORK_CONFIGURE_PROFILE_H_
#define _FRAMEWORK_CONFIGURE_PROFILE_H_

namespace framework
{
    namespace configure
    {

        class Profile
        {
        public:
            // ����һ���յ����ü�
            Profile();

            // �������ļ���ȡ���ü�
            Profile(
                std::string const & file);

            ~Profile();

        public:
            // �������ļ���ȡ���ü�
            int load(
                std::string const & file);

            // �������ļ�д�����ü�
            int save();

            // �������ļ�д�����ü�
            int save(
                std::string const & file);

        public:
            // ��ȡһ���ַ����������ò�����ֵ
            int get(
                std::string const & section, 
                std::string const & key, 
                std::string & val) const;

            // ����һ���ַ����������ò�����ֵ
            int set(
                std::string const & section, 
                std::string const & key, 
                std::string const & val, 
                bool save = true);

            // ����һ���ַ����������ò�����ֵ���������硰ģ��1.���ò���1=����ֵ��
            int pre_set(
                std::string const & line);

            int post_set(
                std::string const & line);

            void get_all(
                std::map<std::string, std::map<std::string, std::string> > & mkvs);

        private:
            std::string file_; // ����������Ϣ���ļ���
            std::map<std::string, std::map<std::string, std::string> > sec_key_vals; // ���ò�����
            std::map<std::string, std::map<std::string, std::string> > pre_sec_key_vals; // ���ò�����
            std::map<std::string, std::map<std::string, std::string> > post_sec_key_vals; // ���ò�����
        };

    } // namespace configure
} // namespace framework

#endif // _FRAMEWORK_CONFIGURE_PROFILE_H_
