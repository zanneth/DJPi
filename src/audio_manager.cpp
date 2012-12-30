/*
 * audio_manager.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#include "audio_manager.h"
#include "logger.h"

#include <iostream>
#include <fmod/fmod_errors.h>
#include <string>

#define MAX_CHANNELS    100

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
    while (!_track_queue.empty()) {
        _track_queue.pop();
    }
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
    _track_queue.push(track);
}

void AudioManager::clear_track_queue()
{
    while (!_track_queue.empty()) {
        _track_queue.pop();
    }
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
                _playing = true;
                _current_track = track;
                _channel = channel;
                
                Logger::Log("Playing track %s...", track->get_filename().c_str());
            }
        } else {
            Logger::Log("Warning: no more tracks in queue.");
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
        _track_queue.push(cmp_track);
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
        enqueue_track(_current_track);
        
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

#pragma mark - Internal

void AudioManager::_print_error(FMOD_RESULT result)
{
    Logger::LogError("FMOD Error %d: %s", result, FMOD_ErrorString(result));
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
        _track_queue.pop();
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
