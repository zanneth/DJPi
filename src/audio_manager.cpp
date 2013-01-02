/*
 * audio_manager.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#include "audio_manager.h"
#include "logger.h"
#include "util.h"

#include <algorithm>
#include <iostream>
#include <fmod/fmod_errors.h>
#include <set>
#include <string>

#define MAX_CHANNELS    100

static FMOD_RESULT F_CALLBACK __channel_callback(FMOD_CHANNEL *channel,
                                                 FMOD_CHANNEL_CALLBACKTYPE type,
                                                 void *commanddata1,
                                                 void *commanddata2);

namespace djpi {

AudioManager::AudioManager() :
    _audio_system(nullptr),
    _channel(nullptr),
    _current_track(nullptr),
    _playing(false)
{
    FMOD_RESULT result = FMOD::System_Create(&_audio_system);
    if (result != FMOD_OK) {
        _print_error(result);
        exit(-1);
    }
    
    result = _audio_system->init(MAX_CHANNELS, FMOD_INIT_NORMAL, NULL);
    if (result != FMOD_OK) {
        _print_error(result);
        exit(-1);
    }
    
    _audio_system->setSpeakerMode(FMOD_SPEAKERMODE_STEREO);
}

AudioManager::~AudioManager()
{
    // release all of our streams first
    clear_track_queue();
    _current_track = nullptr;
    
    if (_channel) {
        _channel->stop();
        _channel = nullptr;
    }
    
    if (_audio_system) {
        _audio_system->release();
        _audio_system = nullptr;
    }
}

#pragma mark - Managing Tracks

void AudioManager::enqueue_track(TrackRef track)
{
    _track_queue.push_back(track);
}

void AudioManager::clear_track_queue()
{
    _track_queue.clear();
}

size_t AudioManager::get_queue_size()
{
    return _track_queue.size();
}

#pragma mark - Controlling Playback

void AudioManager::play()
{
    if (_channel) {
        _channel->setPaused(false);
        _playing = true;
    } else {
        TrackRef track = _current_track;
        if (!track.get()) {
            track = _dequeue_track();
        }
        
        if (track.get()) {
            FMOD::Sound *sound = track->_stream;
            if (!sound) {
                _load_track(track);
                sound = track->_stream;
            }
            
            FMOD::Channel *channel;
            FMOD_RESULT result = _audio_system->playSound(FMOD_CHANNEL_FREE, sound, false, &channel);
            if (result == FMOD_OK) {
                // configure the channel
                channel->setUserData(this);
                channel->setCallback(__channel_callback);
                
                // store related memory
                _playing = true;
                _current_track = track;
                _channel = channel;
                
                // log current track
                std::string track_filename = Util::basename(track->get_filename());
                Logger::log("Playing track %s...", track_filename.c_str());
            }
        } else {
            Logger::log_error("No more tracks in queue.");
        }
    }
}

void AudioManager::pause()
{
    if (_channel) {
        _channel->setPaused(true);
        _playing = false;
    }
}

void AudioManager::stop()
{
    if (_channel) {
        _channel->stop();
        _channel = nullptr;
        _playing = false;
    }
    
    // reset the track queue
    while (!_completed_tracks.empty()) {
        TrackRef cmp_track = _completed_tracks.top(); _completed_tracks.pop();
        _track_queue.push_front(cmp_track);
    }
}

bool AudioManager::is_playing()
{
    return _playing;
}

float AudioManager::get_volume()
{
    float volume = 0.f;
    if (_channel) {
        _channel->getVolume(&volume);
    }
    return volume;
}

void AudioManager::set_volume(float vol)
{
    if (_channel) {
        _channel->setVolume(vol);
    }
}

void AudioManager::next_track()
{
    _complete_current_track();
    
    TrackRef next_track = _dequeue_track();
    if (next_track.get()) {
        _current_track = next_track;
        play();
    }
}

void AudioManager::previous_track()
{
    if (_completed_tracks.size() > 0) {
        // release the stream and enqueue the current track
        pause();
        _current_track->release_stream();
        _channel = nullptr;
        _track_queue.push_front(_current_track);
        
        // pop the the last track and play
        _current_track = _completed_tracks.top(); _completed_tracks.pop();
        play();
    } else {
        stop();
    }
}

#pragma mark - Updating

void AudioManager::update(time_t time)
{
    _audio_system->update();
}

#pragma mark - Callbacks

void AudioManager::track_completion_callback(FMOD::Channel *channel)
{
    if (get_queue_size() > 0) {
        next_track();
    } else {
        _complete_current_track();
    }
}

#pragma mark - Static Methods

bool AudioManager::supports_filename(std::string filename)
{
    static std::set<std::string> __supported_exts = {
        "WAV", "AIFF", "MP3", "OGG", "ASX", "FLAC", "DLS", "ASF", "IT",
        "MP2", "MOD", "RAW", "WAX", "WMA", "XM", "XMA", "S3M", "VAG", "GCADPCM"
    };
    
    std::string extension = Util::filename_ext(filename);
    std::transform(extension.begin(), extension.end(), extension.begin(), ::toupper);
    return __supported_exts.find(extension) != __supported_exts.end();
}

#pragma mark - Internal

void AudioManager::_print_error(FMOD_RESULT result)
{
    Logger::log_error("FMOD Error %d: %s", result, FMOD_ErrorString(result));
}

void AudioManager::_load_track(TrackRef track)
{
    if (track->_stream == nullptr) {
        FMOD::Sound *stream;
        std::string filename = track->get_filename();
        FMOD_RESULT result = _audio_system->createStream(filename.c_str(), FMOD_DEFAULT, NULL, &stream);
        if (result == FMOD_OK) {
            track->_stream = stream;
        } else {
            _print_error(result);
            track->_stream = nullptr;
        }
    }
}

TrackRef AudioManager::_dequeue_track()
{
    TrackRef track = nullptr;
    if (_track_queue.size() > 0) {
        track = _track_queue.front();
        _track_queue.pop_front();
    }
    return track;
}

void AudioManager::_complete_current_track()
{
    if (_current_track.get()) {
        _current_track->release_stream();
        _completed_tracks.push(_current_track);
        _current_track = nullptr;
    }
    
    if (_channel) {
        _channel = nullptr;
    }
}

} // namespace djpi

static FMOD_RESULT F_CALLBACK __channel_callback(FMOD_CHANNEL *channel,
                                                 FMOD_CHANNEL_CALLBACKTYPE type,
                                                 void *commanddata1,
                                                 void *commanddata2)
{
    djpi::AudioManager *audio = nullptr;
    FMOD::Channel *cppchannel = (FMOD::Channel *) channel;
    void *userdata = nullptr;
    
    cppchannel->getUserData(&userdata);
    if (userdata && type == FMOD_CHANNEL_CALLBACKTYPE_END) {
        audio = (djpi::AudioManager *) userdata;
        audio->track_completion_callback(cppchannel);
    }
    
    return FMOD_OK;
}
