/*
 * application.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/27/2012
 */
 
#include "application.h"
#include "audio_manager.h"
#include "input_manager.h"
#include "track.h"
#include <unistd.h>

namespace djpi {

Application::Application(int argc, char **argv) :
    _start_time(0),
    _audio(new AudioManager),
    _input(new InputManager)
{
    for (unsigned i = 0; i < argc; ++i) {
        _arguments.push_back(argv[i]);
    }
}

Application::~Application()
{}

void Application::run()
{
    _start_time = time(NULL);
    
    _enqueue_test_tracks();
    _audio->play();
    
    bool stop = false;
    time_t seconds;
    
    while (!stop) {
        seconds = time(NULL);
        _audio->update(seconds);
        _input->update(seconds);
        usleep(100);
    }
}

#pragma mark - Internal

void Application::_enqueue_test_tracks()
{
    TrackRef track(new Track("test.mp3"));
    _audio->enqueue_track(track);
}

} // namespace djpi
