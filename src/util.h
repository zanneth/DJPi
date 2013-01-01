/*
 * util.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/30/2012
 */
 
#pragma once

#include <string>
#include <vector>

namespace djpi {

class Util {
public:
    static bool is_directory(std::string path);
    static std::vector<std::string> list_directory(std::string path);
    static std::string filename_ext(std::string filename);
    static std::string basename(std::string path);
    static std::string dirname(std::string path);
};

} // namespace djpi
