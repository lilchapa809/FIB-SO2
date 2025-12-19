#ifndef __KEYBOARD_H__
#define __KEYBOARD_H__

// Event structure (defined in the PDF)
struct event_t {
    int pressed;    // 1 if pressed, 0 if released
    char scancode;  // The key scancode
};

// Size of the circular buffer
#define KB_BUFFER_SIZE 16

// Circular buffer structure
struct keyboard_buffer {
    struct event_t events[KB_BUFFER_SIZE];
    int head; // Where we write (Producer)
    int tail; // Where we read (Consumer)
};

// Declaration of functions and global variable
void keyboard_init(void);
extern struct keyboard_buffer k_buffer;

#endif