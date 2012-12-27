/*
 * logger.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#pragma once

#include "logger.h"
#include <cstdarg>
#include <iostream>

namespace djpi {

class Logger {
public:
    static void Log(const char *format, ...);
    static void LogError(const char *format, ...);
    static void Logv(const char *format, va_list args, std::ostream *stream = &std::cout);
};

} // namespace djpi
