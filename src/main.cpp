/*
 * main.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */

#include <iostream>
#include <time.h>
#include <unistd.h>

#include "audio_manager.h"
#include "track.h"

int main(int argc, char **argv)
{
    using namespace djpi;
    
    AudioManager audio;
    TrackRef track(new Track("test.mp3"));
    audio.enqueue_track(track);
    audio.play();
    
    bool stop = false;
    while (!stop) {
        audio.update(time(NULL));
        usleep(100);
    }
    
    return 0;
}
