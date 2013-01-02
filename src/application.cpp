/*
 * application.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/27/2012
 */
 
#include "application.h"
#include "audio_manager.h"
#include "input_manager.h"
#include "logger.h"
#include "track.h"
#include "util.h"

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <unistd.h>

#define HAS_ARG(_X) (std::find(_arguments.begin(), _arguments.end(), _X) != _arguments.end())

static const char *__help =
    "DJPi -- Lightweight MP3 player.\n"
    "Usage: djpi <song directory>\n"
    "Songs in the current directory will be played if no arguments are provided.";

static const char *__header = "=== DJPi ===";

static const char *__controls =
    "Controls:\n"
    "   space   =   pause/play\n"
    "   q       =   quit\n"
    "   left/p  =   previous track\n"
    "   right/n =   next track";

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
    
    _print_header();
    _print_controls();
    
    // parse CLI arguments
    std::vector<std::string> paths;
    bool should_exit = _parse_args(paths);
    if (should_exit) {
        return;
    }
    
    if (paths.size() == 0) {
        Logger::log_error("No tracks were found. Provide a path to song files or place song files in the current directory.");
        return;
    }
    
    // enqueue tracks and start player
    for (auto path : paths) {
        _enqueue_tracks(path);
    }
    _audio->play();
    
    // begin event loop
    time_t seconds;
    while (!_kill_loop) {
        seconds = time(NULL);
        _audio->update(seconds);
        _input->update(seconds);
        
        // check if we're done playing everything
        if (_audio->get_current_track().get() == nullptr && _audio->get_queue_size() == 0) {
            _kill_loop = true;
        }
        
        // poll key events
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

void Application::_print_controls()
{
    Logger::log(__controls);
}

void Application::_print_header()
{
    Logger::log(__header);
}

bool Application::_parse_args(std::vector<std::string> &paths)
{
    bool should_exit = false;
    if (HAS_ARG("--help")) {
        std::cerr << __help;
        should_exit = true;
    }
    
    if (!should_exit) {
        for (auto itr = _arguments.begin() + 1; itr != _arguments.end(); ++itr) {
            std::string arg = *itr;
            if (arg[0] != '-') {
                paths.push_back(arg);
            }
        }
        
        if (paths.size() == 0) {
            char cwdbuf[256];
            getcwd(cwdbuf, 256);
            paths.push_back(cwdbuf);
        }
    }
    
    return should_exit;
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
        case 0x43: // right arrow
        case 'n':
            _audio->next_track();
            break;
        case 0x44: // left arrow
        case 'p':
            _audio->previous_track();
            break;
        default:
            break;
    }
}

void Application::_enqueue_tracks(std::string path)
{
    std::vector<std::string> track_filenames;
    
    if (Util::is_directory(path)) {
        std::vector<std::string> dir_contents = Util::list_directory(path);
        for (auto filename : dir_contents) {
            if (!Util::is_directory(path + "/" + filename)) {
                if (AudioManager::supports_filename(filename)) {
                    track_filenames.push_back(filename);
                } else {
                    Logger::log_error("Warning: %s is an unsupported file type.", filename.c_str());
                }
            }
        }
    } else if (AudioManager::supports_filename(path)) {
        track_filenames.push_back(Util::basename(path));
        path = Util::dirname(path);
    } else {
        Logger::log_error("Warning: %s is an unsupported file type.", path.c_str());
    }
    
    Logger::log("Playlist (%d total tracks):", track_filenames.size());
    for (auto track_filename : track_filenames) {
        Logger::log("\t%s", track_filename.c_str());
        std::string abspath = path + "/" + track_filename;
        TrackRef track(new Track(abspath));
        _audio->enqueue_track(track);
    }
}

} // namespace djpi
