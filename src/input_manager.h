/*
 * input_manager.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#pragma once

#include <functional>
#include <time.h>

namespace djpi {

struct KeyEvent {
    unsigned char key;
    time_t timestamp;
};

typedef std::function<void(KeyEvent)> InputCallback;

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    // update
    void update(time_t time);
    
    // setting up event handlers
    void set_callback(InputCallback callback) { _callback = callback; }
    
private:
    void _setup_terminal();
    void _restore_terminal();
    
protected:
    InputCallback _callback;
};
    
} // namespace djpi
