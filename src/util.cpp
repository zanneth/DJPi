/*
 * util.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/30/2012
 */
 
#include "util.h"
#include <cassert>
#include <cstdio>
#include <cstring>
#include <dirent.h>
#include <libgen.h>
#include <sys/stat.h>
#include <sys/types.h>

namespace djpi {

bool Util::is_directory(std::string path)
{
    bool is_directory = false;
    
    struct stat s;
    int result = stat(path.c_str(), &s);
    if (result == 0) {
        is_directory = S_ISDIR(s.st_mode);
    }
    
    return is_directory;
}

std::vector<std::string> Util::list_directory(std::string path)
{
    std::vector<std::string> files;
    DIR *dirptr;
    struct dirent *entry;
    
    dirptr = opendir(path.c_str());
    if (dirptr != nullptr) {
        while ((entry = readdir(dirptr))) {
            char *filename = entry->d_name;
            if (filename[0] != '.' && strlen(filename) > 0) {
                files.push_back(filename);
            }
        }
        closedir(dirptr);
    } else {
        throw "Cannot list directory contents.";
    }
    
    return files;
}

std::string Util::filename_ext(std::string filename)
{
    std::string extension;
    
    const char *filename_str = filename.c_str();
    const char *dot_occur = strrchr(filename_str, '.');
    if (dot_occur == nullptr || dot_occur == filename_str) {
        extension = "";
    } else {
        extension = dot_occur + 1;
    }
    
    return extension;
}

std::string Util::basename(std::string path)
{
    std::string base;
    char *pathstr = strdup(path.c_str());
    char *basestr = ::basename(pathstr);
    if (basestr != NULL) {
        base = basestr;
    }
    
    return basestr;
}

} // namespace djpi
