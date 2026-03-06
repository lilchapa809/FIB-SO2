/*
 * interrupt.c - IDT init and Service Routines for Interruptiosn and Exceptions
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

/** 
 * Definition of zeos_ticks
 */
int zeos_ticks;


char char_map[] =
{
  '\0','\0','1','2','3','4','5','6',
  '7','8','9','0','\'','�','\0','\0',
  'q','w','e','r','t','y','u','i',
  'o','p','`','+','\0','\0','a','s',
  'd','f','g','h','j','k','l','�',
  '\0','�','\0','�','z','x','c','v',
  'b','n','m',',','.','-','\0','*',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0','\0','\0','\0','\0','\0','7',
  '8','9','-','4','5','6','+','1',
  '2','3','0','\0','\0','\0','<','\0',
  '\0','\0','\0','\0','\0','\0','\0','\0',
  '\0','\0'
};

void setInterruptHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE INTERRUPTION GATE FLAGS:                          R1: pg. 5-11  */
  /* ***************************                                         */
  /* flags = x xx 0x110 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void setTrapHandler(int vector, void (*handler)(), int maxAccessibleFromPL)
{
  /***********************************************************************/
  /* THE TRAP GATE FLAGS:                                  R1: pg. 5-11  */
  /* ********************                                                */
  /* flags = x xx 0x111 000 ?????                                        */
  /*         |  |  |                                                     */
  /*         |  |   \ D = Size of gate: 1 = 32 bits; 0 = 16 bits         */
  /*         |   \ DPL = Num. higher PL from which it is accessible      */
  /*          \ P = Segment Present bit                                  */
  /***********************************************************************/
  Word flags = (Word)(maxAccessibleFromPL << 13);

  //flags |= 0x8F00;    /* P = 1, D = 1, Type = 1111 (Trap Gate) */
  /* Changed to 0x8e00 to convert it to an 'interrupt gate' and so
     the system calls will be thread-safe. */
  flags |= 0x8E00;    /* P = 1, D = 1, Type = 1110 (Interrupt Gate) */

  idt[vector].lowOffset       = lowWord((DWord)handler);
  idt[vector].segmentSelector = __KERNEL_CS;
  idt[vector].flags           = flags;
  idt[vector].highOffset      = highWord((DWord)handler);
}

void hex_convertor(unsigned int address, char *s) 
{
  int i;
  s[8] = '\0';  // Null terminate at position 8 for 8 hex digits
  for (i = 7; i >= 0; i--) {
    int digit = address % 16;
    if (digit < 10) {
      s[i] = '0' + digit;  // Convert 0-9 to '0'-'9'
    } else {
      s[i] = 'A' + digit - 10;  // Convert 10-15 to 'A'-'F'
    }
    address /= 16;
  }
}

void clock_routine()
{
  ++zeos_ticks;
  zeos_show_clock(); 
}

void keyboard_routine(){
  unsigned char kb_reg = inb(0x60); //code from keyboard
  unsigned char is_make = kb_reg & 0x80; //is make or break
  unsigned char code = kb_reg & 0x7F; //code of the key
  if(!is_make){
    char c = char_map[code];
    if(c != 0) printc_xy(0x00,0x00,c); //print the character
    else printc_xy(0x00,0x00,'C'); //print 'C' if the character is not found
  }
}

// Kernel Stack at this point: error, eip, cs, eflags, esp, ss. 
// We are only interested in error and eip, so we will ignore the rest of the parameters.
void my_page_fault_routine(unsigned int error, unsigned int eip)
{
  char s[16];

  // Display the instruction pointer that caused the page fault
  hex_convertor(eip, s);
  printk("Process generates a PAGE FAULT exception\n");
  printk("    EIP: 0x"); 
  printk(s);
  printc('\n');
  
  // Display the page fault error code
  hex_convertor(error, s);
  printk("    Error Code: 0x");
  printk(s);
  printc('\n');

  while(1); // Halt the system to prevent further damage 
}

void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();

  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */

  //Clock Interrupt
  setInterruptHandler(32, clock_handler, 0); //We must declare clock_handler in interrupt.h
  //Keyboard Interrupt
  setInterruptHandler(33, keyboard_handler, 0);

  //Page Fault Interrupt
  setInterruptHandler(14, my_page_fault_handler, 0);

  //Syscall Handler, 3 --> User Level Privilege
  setTrapHandler(0x80, system_call_handler, 3);
  set_idt_reg(&idtR);
}

