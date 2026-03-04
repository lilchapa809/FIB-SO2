/*
 * interrupt.h - Definició de les diferents rutines de tractament d'exepcions
 */

#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__

#include <types.h>

#define IDT_ENTRIES 256

extern Gate idt[IDT_ENTRIES];
extern Register idtR;

/** Declaration 
 * zeos_ticks: Global tick system counter.
 * Incremented everytime the clock interrupt triggers.
 * Used by sys_gettime();
 * Defined in system.c
 */
extern int zeos_ticks;

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL);
void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL);

void clock_handler();
void system_call_handler();

void keyboard_handler();

void setIdt();

#endif  /* __INTERRUPT_H__ */
