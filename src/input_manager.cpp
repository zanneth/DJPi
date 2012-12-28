/*
 * input_manager.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/27/2012
 */
 
#include "input_manager.h"
#include <cassert>
#include <cstring>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

#define MAX_EVENT_QUEUE_SIZE    100

static bool __terminal_configured = false;
static struct termios __original_opts;

namespace djpi {

InputManager::InputManager()
{
    if (!__terminal_configured) {
        _setup_terminal();
    }
}

InputManager::~InputManager()
{
    if (__terminal_configured) {
        _restore_terminal();
    }
}

#pragma mark - Updating

void InputManager::update(time_t time)
{
    struct timeval timeout = {0};
    fd_set fds;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds);
    
    int ready = select(1, &fds, NULL, NULL, &timeout);
    if (ready) {
        unsigned char c;
        read(STDIN_FILENO, &c, sizeof(c));
        
        KeyEvent e = { c, time };
        _enqueue_event(e);
    }
}

#pragma mark - Polling the Event Queue

bool InputManager::poll_event(KeyEvent *event_out)
{
    bool has_event = false;
    if (_event_queue.size() > 0) {
        has_event = true;
        KeyEvent evnt = _event_queue.front();
        _event_queue.pop_front();
        *event_out = evnt;
    }
    return has_event;
}

#pragma mark - Internal

void InputManager::_setup_terminal()
{
    struct termios new_opts;
    tcgetattr(STDIN_FILENO, &__original_opts);
    memcpy(&new_opts, &__original_opts, sizeof(new_opts));
    new_opts.c_lflag &= ~(ICANON | ECHO | ECHOE | ECHOK | ECHONL | ECHOPRT | ECHOKE | ICRNL);
    tcsetattr(STDIN_FILENO, TCSANOW, &new_opts);
    
    __terminal_configured = true;
}

void InputManager::_restore_terminal()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &__original_opts);
    __terminal_configured = false;
}

void InputManager::_enqueue_event(const KeyEvent &event)
{
    if (_event_queue.size() >= MAX_EVENT_QUEUE_SIZE) {
        _event_queue.pop_front();
    }
    
    _event_queue.push_back(event);
}
    
} // namespace djpi
