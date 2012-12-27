/*
 * logger.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */

#include "logger.h"
#include <cstdio>

#define BUFFER_SIZE 1024

namespace djpi {

void Logger::Log(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Logv(format, args);
    va_end(args);
}

void Logger::LogError(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    Logv(format, args, &std::cerr);
    va_end(args);
}

void Logger::Logv(const char *format, va_list args, std::ostream *stream)
{
    char buf[BUFFER_SIZE];
    vsprintf(buf, format, args);
    
    (*stream) << buf << std::endl;
}

} // namespace djpi
