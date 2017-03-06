// main.cpp

#include <framework/Framework.h>
#include <framework/system/Version.h>

#include <iostream>

int main(int argc, char * argv[])
{
    if (argc != 2 && argc != 3 && argc != 4) {
        std::cout << "usage: file_version file [module] [version]" << std::endl;
        return 1;
    }

    std::string file = argv[1];
    boost::system::error_code ec;

    if (argc == 2) {
	std::map<std::string, std::string> module_versions;
        ec = framework::system::Version::get_version(file, module_versions);
        if (!ec) {
            for (std::map<std::string, std::string>::const_iterator iter = module_versions.begin(); iter != module_versions.end(); ++iter) {
                std::cout << iter->first << ": " << iter->second << std::endl;
            }
        }
    } else if (argc == 3) {
    	std::string module = argv[2];
        std::string version;
        ec = framework::system::Version::get_version(file, module, version);
        if (!ec) {
            std::cout << version << std::endl;
        }
    } else {
    	std::string module = argv[2];
        std::string version = argv[3];
        ec = framework::system::Version::set_version(file, module, version);
    }
    if (ec) {
        std::cerr << file << ": " << ec.message() << std::endl;
    }
    return ec.value();
}
