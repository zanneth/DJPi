/*
 * audio_manager.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#pragma once

#include <cstring>
#include <fmod/fmod.hpp>
#include <deque>
#include <stack>
#include <time.h>
#include "track.h"

namespace djpi {

class AudioManager {
public:
    AudioManager();
    ~AudioManager();
    
    // managing tracks
    void enqueue_track(TrackRef track);
    void clear_track_queue();
    size_t get_queue_size();
    TrackRef get_current_track() const { return _current_track; }
    
    // controlling playback
    void play();
    void pause();
    void stop();
    bool is_playing();
    float get_volume();
    void set_volume(float vol); // 0.0 - 1.0
    void next_track();
    void previous_track();
    
    // updating
    void update(time_t time);
    
    // callbacks
    void track_completion_callback(FMOD::Channel *channel);
    
    // static methods
    static bool supports_filename(std::string filename);
    
private:
    void _print_error(FMOD_RESULT result);
    void _load_track(TrackRef track);
    TrackRef _dequeue_track();
    void _complete_current_track();

protected:
    FMOD::System *_audio_system;
    FMOD::Channel *_channel;
    std::deque<TrackRef> _track_queue;
    std::stack<TrackRef> _completed_tracks;
    TrackRef _current_track;
    bool _playing;
};

} // namespace djpi
