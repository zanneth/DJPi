/*
 * track.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#pragma once

#include <fmod/fmod.hpp>
#include <memory>
#include <string>

namespace djpi {

class Track {
public:
    Track(std::string filename = "");
    Track(const Track&) = delete;
    ~Track();
    
    // accessors
    std::string get_filename() const { return _filename; }
    
    void release_stream();

protected:
    std::string _filename;
    FMOD::Sound *_stream;

    friend class AudioManager;
};

typedef std::shared_ptr<Track> TrackRef;
    
} // namespace djpi
