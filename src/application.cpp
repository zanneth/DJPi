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
    _input(new InputManager),
    _kill_loop(false)
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
    
    time_t seconds;
    while (!_kill_loop) {
        seconds = time(NULL);
        _audio->update(seconds);
        _input->update(seconds);
        
        KeyEvent event;
        while (_input->poll_event(&event)) {
            _handle_event(event);
        }
        
        usleep(100);
    }
}

void Application::quit()
{
    _kill_loop = true;
}

#pragma mark - Internal

void Application::_enqueue_test_tracks()
{
    TrackRef track(new Track("test.mp3"));
    _audio->enqueue_track(track);
}

void Application::_handle_event(const KeyEvent &e)
{
    switch (e.key) {
        case 'q':
            quit();
            break;
        case 0x20: // space
            if (_audio->is_playing()) {
                _audio->pause();
            } else {
                _audio->play();
            }
            break;
        default:
            break;
    }
}

} // namespace djpi
