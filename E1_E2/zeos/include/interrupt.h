/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

/**
 * @brief Set up an interrupt gate in the IDT
 * @param vector Interrupt vector number (0-255)
 * @param handler Pointer to the interrupt handler function
 * @param maxAccessibleFromPL Maximum privilege level that can access this interrupt
 *                            (0 = kernel only, 3 = user space)
 * 
 * Used for hardware interrupts and exceptions that should disable interrupts
 * during execution (IF flag cleared)
 */
void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

/**
 * @brief Set up a trap gate in the IDT  
 * @param vector Interrupt vector number (0-255)
 * @param handler Pointer to the trap handler function
 * @param maxAccessibleFromPL Maximum privilege level that can access this trap
 *                            (0 = kernel only, 3 = user space)
 * 
 * Used for system calls and exceptions that should preserve interrupts
 * during execution (IF flag unchanged)
 */
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

/**
 * @brief Initialize the Interrupt Descriptor Table (IDT)
 * 
 * Sets up all exception handlers, interrupt handlers, and system call handler.
 * Must be called during OS initialization before enabling interrupts.
 */
void setIdt();

/* === Interrupt Service Routine Handlers (defined in entry.S) === */

/**
 * @brief Keyboard interrupt handler (Interrupt 33)
 * 
 * Called when a keyboard key is pressed/released.
 * Reads scan code from port 0x60 and processes the input.
 */
void keyboard_handler();

/**
 * @brief Clock interrupt handler (Interrupt 32) 
 * 
 * Called on each timer tick. Increments zeos_ticks counter
 * and updates the on-screen clock display.
 */
void clock_handler();

/**
 * @brief Page fault exception handler (Interrupt 14)
 * 
 * Called when a page fault occurs. Prints debug information
 * about the faulting address and enters infinite loop since
 * ZeOS doesn't implement page fault recovery.
 */
void my_page_fault_handler();

/**
 * @brief System call handler (Interrupt 0x80)
 * 
 * Entry point for all system calls from user space.
 * Dispatches to the appropriate sys_* function based on
 * the system call number in EAX register.
 */
void system_call_handler();

void syscall_handler_sysenter();


#endif  /* __INTERRUPT_H__ */
