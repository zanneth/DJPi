/*
 * track.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#include "track.h"

namespace djpi {

Track::Track(std::string filename) :
    _filename(filename),
    _stream(nullptr)
{}

Track::~Track()
{
    if (_stream) {
        _stream->release();
        _stream = nullptr;
    }
}

} // namespace djpi
