// main.cpp

#include <framework/Framework.h>
#include <framework/system/FileTag.h>

#include <iostream>

int main(int argc, char * argv[])
{
    if (argc < 3) {
        std::cout << "usage: file_tag file type [ name | name=value ... ]" << std::endl;
        return 1;
    }

    std::string file = argv[1];
    std::string type = argv[2];

    boost::system::error_code ec;

    std::map<std::string, std::string> get_values;
    std::map<std::string, std::string> set_values;

    for (int i = 3; i < argc; ++i) {
        char * equal = strchr(argv[i], '=');
        if (equal) {
            *equal++ = 0;
            set_values[argv[i]] = equal;
        } else {
            get_values[argv[i]] = "";
        }
    }

    if (!get_values.empty() || set_values.empty()) {
        framework::system::FileTag ft(file, type);
        ec = ft.get_all(get_values);
        for (std::map<std::string, std::string>::const_iterator iter = get_values.begin(); iter != get_values.end(); ++iter) {
            std::cout << iter->first << ": " << iter->second << std::endl;
        }
        if (ec) {
            std::cerr << file << ": " << ec.message() << std::endl;
        }
    }

    if (!set_values.empty()) {
        framework::system::FileTag ft(file, type);
        ec = ft.set_all(set_values);
        for (std::map<std::string, std::string>::const_iterator iter = set_values.begin(); iter != set_values.end(); ++iter) {
            std::cout << iter->first << ": " << iter->second << std::endl;
        }
        if (ec) {
            std::cerr << file << ": " << ec.message() << std::endl;
        }
    }

    return ec.value();
}
