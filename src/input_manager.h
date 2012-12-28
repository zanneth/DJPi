/*
 * input_manager.h
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/27/2012
 */
 
#pragma once

#include <deque>
#include <functional>
#include <time.h>

namespace djpi {

struct KeyEvent {
    unsigned char key;
    time_t timestamp;
};

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    // update
    void update(time_t time);
    
    // polling the event queue
    bool poll_event(KeyEvent *event_out);
    
private:
    void _setup_terminal();
    void _restore_terminal();
    void _enqueue_event(const KeyEvent &event);
    
protected:
    std::deque<KeyEvent> _event_queue;
};
    
} // namespace djpi
