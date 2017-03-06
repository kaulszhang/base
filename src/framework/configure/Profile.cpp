// Profile.cpp

#include "framework/Framework.h"
#include "framework/configure/Profile.h"
#include "framework/string/StringToken.h"
#include "framework/string/Algorithm.h"

#include <fstream>
#include <iterator>

namespace framework
{
    namespace configure
    {

        using framework::string::trim;
        using framework::string::string_limit;

        Profile::Profile()
        {
        }

        Profile::Profile(
            std::string const & file)
        {
            load(file);
        }

        Profile::~Profile()
        {
        }

        int Profile::load(
            std::string const & file)
        {
            file_ = file;

            std::string section;
            std::map<std::string, std::string> key_vals;
            std::ifstream ifs(file.c_str());
            std::string line;

            if (!ifs)
                return 1;

            while (std::getline(ifs, line)) {
                trim(line);
                if (line.empty() || line[0] == '#')
                    continue;
                if (line[0] == '[') {
                    if (line[line.length() - 1] == ']') {
                        if (!section.empty())
                            sec_key_vals[section] = key_vals;
                        key_vals.clear();
                        section = line.substr(1, line.length() - 2);
                    }
                } else {
                    std::string::size_type pos_equal = line.find('=');
                    if (pos_equal != std::string::npos) {
                        std::string key = line.substr(0, pos_equal);
                        trim(key);
                        std::string val = line.substr(pos_equal + 1);
                        trim(val);
                        key_vals[key] = val;
                    }
                }
            }
            if (!section.empty())
                sec_key_vals[section] = key_vals;

            return 0;
        }

        int Profile::save()
        {
            return save(file_);
        }

        int Profile::save(
            std::string const & file)
        {
            std::ifstream ifs(file.c_str(), std::ios::binary);
            ifs >> std::noskipws;
            std::istream_iterator<char> beg(ifs), end;
            //std::string text(beg, end);
            std::string text;
            std::copy(beg, end, std::inserter(text, text.end()));
            std::string text2;

            std::map<std::string, std::map<std::string, std::string> > temp = sec_key_vals;
            std::string section;
            std::map<std::string, std::string> key_vals;

            std::string::size_type p = 0;
            std::string::size_type sp = 0;
            while (1) {
                if (p >= text.size())
                    break;
                std::string::size_type q = text.find('\n', p);
                if (q == std::string::npos)
                    q = text.size();
                // 如果是注释行，跳过
                if (text[p] == '#') {
                    p = q + 1;
                    continue;
                }
                // 去除行两端的空白
                string_limit line_limit(p, q);
                trim(text, line_limit);
                // 如果是空行，跳过
                if (line_limit.second == line_limit.first) {
                    p = q + 1;
                    continue;
                }
                // 如果是Section开始行
                if (text[line_limit.first] == '[') {
                    if (text[line_limit.second - 1] == ']') {
                        // 加入上一个Section未定义的Key
                        if (!section.empty() && !key_vals.empty()) {
                            text2 += text.substr(sp, p - sp);
                            sp = p;
                            for (std::map<std::string, std::string>::const_iterator k = key_vals.begin(); k != key_vals.end(); ++k) {
                                text2 += k->first;
                                text2 += '=';
                                text2 += k->second;
                                text2 += "\r\n";
                            }
                        }
                        section = text.substr(line_limit.first + 1, line_limit.second - line_limit.first - 2);
                        key_vals = temp[section];
                        // 该Section是存在的，从temp中删除
                        temp.erase(section);
                    }
                } else {
                    std::string::size_type pos_equal = text.find('=', line_limit.first);
                    if (pos_equal != std::string::npos && pos_equal < line_limit.second) {
                        std::string key = text.substr(line_limit.first, pos_equal - line_limit.first);
                        trim(key);
                        string_limit value_limit(pos_equal + 1, line_limit.second);
                        trim(text, value_limit);
                        text2 += text.substr(sp, value_limit.first - sp);
                        //sp = value_limit.first;
                        text2 += key_vals[key];
                        sp = value_limit.second;
                        key_vals.erase(key);
                    }
                }
                p = q + 1;
            }

            text2 += text.substr(sp);

            if (p > text.size())
                text2 += "\r\n";

            // 加入上一个Section未定义的Key
            if (!section.empty() && !key_vals.empty()) {
                for (std::map<std::string, std::string>::const_iterator k = key_vals.begin(); k != key_vals.end(); ++k) {
                    text2 += k->first;
                    text2 += '=';
                    text2 += k->second;
                    text2 += "\r\n";
                }
            }

            std::ofstream ofs(file.c_str(), std::ios::binary);
            if (!ofs)
                return 1;

            ofs << text2;

            for (std::map<std::string, std::map<std::string, std::string> >::const_iterator s = temp.begin(); s != temp.end();++ s) {
                ofs << '[' << s->first << ']' << "\r\n";
                for (std::map<std::string, std::string>::const_iterator k = s->second.begin(); k != s->second.end(); ++k) {
                    ofs << k->first << '=' << k->second << "\r\n";
                }
            }

            return 0;
        }

        int Profile::get(
            std::string const & sec, 
            std::string const & key, 
            std::string & val) const
        {
            std::map<std::string, std::map<std::string, std::string> >::const_iterator s = pre_sec_key_vals.find(sec);
            if (s != pre_sec_key_vals.end()) {
                std::map<std::string, std::string>::const_iterator k = s->second.find(key);
                if (k != s->second.end()) {
                    val = k->second;
                    return 0;
                }
            }
            s = sec_key_vals.find(sec);
            if (s != sec_key_vals.end()) {
                std::map<std::string, std::string>::const_iterator k = s->second.find(key);
                if (k != s->second.end()) {
                    val = k->second;
                    return 0;
                }
            }
            s = post_sec_key_vals.find(sec);
            if (s != post_sec_key_vals.end()) {
                std::map<std::string, std::string>::const_iterator k = s->second.find(key);
                if (k != s->second.end()) {
                    val = k->second;
                    return 0;
                }
            }
            return 1;
        }

        int Profile::set(
            std::string const & section, 
            std::string const & key, 
            std::string const & val, 
            bool b_save)
        {
            sec_key_vals[section][key] = val;
            if (!b_save)
                return 0;

            std::ifstream ifs(file_.c_str(), std::ios::binary);
            ifs >> std::noskipws;
            std::istream_iterator<char> beg(ifs), end;
            //std::string text(beg, end);
            std::string text;
            std::copy(beg, end, std::inserter(text, text.end()));
            std::string text2;

            bool section_found = false;
            bool key_found = false;

            std::string::size_type p = 0;
            std::string::size_type sp = 0;
            while (1) {
                if (p >= text.size())
                    break;
                std::string::size_type q = text.find('\n', p);
                if (q == std::string::npos)
                    q = text.size();
                // 如果是注释行，跳过
                if (text[p] == '#') {
                    p = q + 1;
                    continue;
                }
                // 去除行两端的空白
                string_limit line_limit(p, q);
                trim(text, line_limit);
                // 如果是空行，跳过
                if (line_limit.second == line_limit.first) {
                    p = q + 1;
                    continue;
                }
                // 如果是Section开始行
                if (text[line_limit.first] == '[') {
                    if (text[line_limit.second - 1] == ']') {
                        if (section_found) {
                            key_found = true;
                            text2 += text.substr(sp, p - sp);
                            sp = p;
                            text2 += key;
                            text2 += '=';
                            text2 += val;
                            text2 += "\r\n";
                            break;
                        }
                        section_found = (section == text.substr(line_limit.first + 1, line_limit.second - line_limit.first - 2));
                    }
                } else if (section_found) {
                    std::string::size_type pos_equal = text.find('=', line_limit.first);
                    if (pos_equal != std::string::npos && pos_equal < line_limit.second) {
                        std::string key1 = text.substr(line_limit.first, pos_equal - line_limit.first);
                        trim(key1);
                        if (key == key1) {
                            key_found = true;
                            string_limit value_limit(pos_equal + 1, line_limit.second);
                            trim(text, value_limit);
                            text2 += text.substr(sp, value_limit.first - sp);
                            text2 += val;
                            sp = value_limit.second;
                            break;
                        }
                    }
                }
                p = q + 1;
            }

            text2 += text.substr(sp);

            if (p > text.size())
                text2 += "\r\n";

            if (!section_found) {
                text2 += "[";
                text2 += section;
                text2 += "]\r\n";
            }

            if (!key_found) {
                text2 += key;
                text2 += '=';
                text2 += val;
                text2 += "\r\n";
            }

            std::ofstream ofs(file_.c_str(), std::ios::binary);
            if (!ofs)
                return 1;

            ofs << text2;

            return 0;
        }

        int Profile::pre_set(
            std::string const & line)
        {
            std::string::size_type pos_eq = line.find('=');
            if (pos_eq == std::string::npos)
                return 1;
            std::string::size_type pos_dot = line.substr(0, pos_eq).find('.');
            if (pos_dot == std::string::npos)
                return 2;
            pre_sec_key_vals[line.substr(0, pos_dot)][line.substr(pos_dot + 1, pos_eq - pos_dot - 1)] = line.substr(pos_eq + 1);
            return 0;
        }

        int Profile::post_set(
            std::string const & line)
        {
            std::string::size_type pos_eq = line.find('=');
            if (pos_eq == std::string::npos)
                return 1;
            std::string::size_type pos_dot = line.substr(0, pos_eq).find('.');
            if (pos_dot == std::string::npos)
                return 2;
            post_sec_key_vals[line.substr(0, pos_dot)][line.substr(pos_dot + 1, pos_eq - pos_dot - 1)] = line.substr(pos_eq + 1);
            return 0;
        }

        void Profile::get_all(
            std::map<std::string, std::map<std::string, std::string> > & mkvs)
        {
            mkvs = sec_key_vals;
        }
    }
}
