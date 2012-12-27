/*
 * main.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */

#include <iostream>
#include <memory>
#include <time.h>
#include <unistd.h>

#include "audio_manager.h"
#include "input_manager.h"
#include "track.h"

int main(int argc, char **argv)
{
    using namespace djpi;
    
    std::shared_ptr<AudioManager> audio(new AudioManager);
    std::shared_ptr<InputManager> input(new InputManager);
    
    bool stop = false;
    time_t seconds = time(NULL);
    
    input->set_callback([&stop, audio](KeyEvent e) {
        switch (e.key) {
            case 'q':
                stop = true;
                break;
            case 0x20: // space
                if (audio->is_playing()) {
                    audio->pause();
                } else {
                    audio->play();
                }
                break;
            default:
                break;
        }
    });
    
    TrackRef track(new Track("test.mp3"));
    audio->enqueue_track(track);
    audio->play();
    
    while (!stop) {
        audio->update(seconds);
        input->update(seconds);
        usleep(100);
    }
    
    return 0;
}
