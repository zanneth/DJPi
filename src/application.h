/*
 * application.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/27/2012
 */
 
#pragma once

#include <ctime>
#include <memory>
#include <string>
#include <vector>

namespace djpi {

class AudioManager;
class InputManager;

class Application {
public:
    Application(int argc, char **argv);
    ~Application();
    
    // accessors
    std::shared_ptr<AudioManager> get_audio() const { return _audio; }
    std::shared_ptr<InputManager> get_input() const { return _input; }
    
    // running
    void run();
    
private:
    void _enqueue_test_tracks();
    
protected:
    std::vector<std::string> _arguments;
    time_t _start_time;
    std::shared_ptr<AudioManager> _audio;
    std::shared_ptr<InputManager> _input;
};

} // namespace djpi