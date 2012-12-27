/*
 * input_manager.cpp
 *
 * Author: Charles Magahern <charles@magahern.com>
 * Date Created: 12/26/2012
 */
 
#include "input_manager.h"
#include <cassert>
#include <cstring>
#include <sys/select.h>
#include <termios.h>
#include <unistd.h>

static bool __terminal_configured = false;
static struct termios __original_opts;

namespace djpi {

InputManager::InputManager() :
    _callback([](KeyEvent){})
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
        _callback(e);
    }
}

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
    
} // namespace djpi
