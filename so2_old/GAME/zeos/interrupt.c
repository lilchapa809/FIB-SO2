/* interrupt.c - */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>
#include <sched.h>
#include <zeos_interrupt.h>

// Include the new keyboard header
#include <keyboard.h> 

Gate idt[IDT_ENTRIES];
Register idtR;

char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','¡','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','ñ',
  '\0','º','\0','ç','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

int zeos_ticks = 0;

// Global keyboard buffer instance
char keyboard_buffer[128];  // Vector of the possible keys -> 1 if pressed 0 if not.

// Initialize the keyboard buffer
void keyboard_init()
{
    k_buffer.head = 0;
    k_buffer.tail = 0;
}

// Keyboard interrupt service routine
void keyboard_routine()
{
  unsigned char c = inb(0x60); // Read scancode from port 0x60
  int make;
  
  // Check if it is a Make (press) or Break (release) code
  if (c & 0x80) { // If bit 7 is 1, it is a Break code
      make = 0;
      c &= 0x7F; // Clear bit 7 to get the raw scancode
  } else {
      make = 1;
  }

  // Calculate the next head position
  int next_head = (k_buffer.head + 1) % KB_BUFFER_SIZE;

  // Circular Buffer Logic:
  // If next_head equals tail, the buffer is full. Drop the event.
  if (next_head != k_buffer.tail) {
      // Store the event in the buffer
      k_buffer.events[k_buffer.head].scancode = c;
      k_buffer.events[k_buffer.head].pressed = make;
      
      // Update the head pointer
      k_buffer.head = next_head;
  }
}

void clock_routine()
{
  zeos_show_clock();
  zeos_ticks ++;

  // === TEMPORARY TEST FOR MILESTONE 1 ===
  // We act as a consumer to verify the buffer works
  if (k_buffer.head != k_buffer.tail) {
      
      // 1. Get the event from the tail
      struct event_t ev = k_buffer.events[k_buffer.tail];
      
      // 2. Advance the tail (consume the event)
      k_buffer.tail = (k_buffer.tail + 1) % KB_BUFFER_SIZE;
      
      // 3. Visual feedback: Print if pressed
      if (ev.pressed) {
          char c = char_map[(unsigned char)ev.scancode];
          
          if (c != '\0') {
              printk("Buffer OK: ");
              printc(c);
              printc('\n');
          } else {
              printk("Buffer OK: [Special Key]\n");
          }
      }
  }
  // === END OF TEST ===
  
  schedule();
}

// ... (Resto del archivo: setInterruptHandler, setIdt, etc. se mantienen igual) ...
void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;   

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void clock_handler();
void keyboard_handler();
void system_call_handler();

void setMSR(unsigned long msr_number, unsigned long high, unsigned long low);

void setSysenter()
{
  setMSR(0x174, 0, __KERNEL_CS);
  setMSR(0x175, 0, INITIAL_ESP);
  setMSR(0x176, 0, (unsigned long)system_call_handler);
}

void setIdt()
{
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(33, keyboard_handler, 0);

  setSysenter();

  set_idt_reg(&idtR);
}