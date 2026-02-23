/*
 * interrupt.c -
 */
#include <types.h>
#include <interrupt.h>
#include <segment.h>
#include <hardware.h>
#include <io.h>

#include <zeos_interrupt.h>

Gate idt[IDT_ENTRIES];
Register    idtR;

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

/** @brief Ticks since the system started */ 
int zeos_ticks = 0;

extern struct task_struct *idle_task;
extern struct task_struct *task1;

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



/** 
* @brief Converts an unsigned integer to a hexadecimal string 
* @param address Unsigned integer that we want to convert
* @param s String that will contain each digit of address
*/
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

/**
 * @brief Keyboard interrupt service routine
 * 
 * Handles keyboard input by reading the keyboard data port (0x60),
 * distinguishing between key press (make) and release (break) events,
 * and displaying the corresponding character on screen.
 * 
 * If an unmapped key is pressed, clears the entire screen and displays 'C'.
 */
void keyboard_routine()
{
  // Screen dimensions: 80 columns (0x4F) x 25 rows (0x18)
  unsigned char kb_register = inb(0x60); // Read scan code from keyboard data port
  unsigned char is_make = kb_register & 0x80; //kb_register[7] --> mask applied 1000 0000
  unsigned char code = kb_register & 0x7F; //kb_register[6..0] --> mask applied 0111 1111
  if (!is_make){
    char to_print = char_map[code];
    if (to_print != 0) printc_xy(0x00,0x00,to_print);
    
    else {
    printc_xy(0x00,0x00,'C');
    }
  }
}

/**
 * @brief Clock interrupt service routine
 * 
 * Handles timer ticks by incrementing the global tick counter
 * and updating the on-screen clock display.
 * Note: Time in Bochs emulator runs faster than real time.
 */
void clock_routine()
{
  //Time goes faster than the real one
  ++zeos_ticks;
  zeos_show_clock();
}

/**
 * @brief Page Fault exception service routine
 * @param error The error code provided by the CPU describing the fault type
 * @param eip The instruction pointer value when the page fault occurred
 * 
 * Handles page fault exceptions by displaying diagnostic information
 * about the faulting instruction and error code, then enters an
 * infinite loop since ZeOS doesn't implement page fault recovery.
 * 
 * The faulting address could be retrieved from CR2 register if needed.
 */
void my_page_fault_routine(unsigned int error, unsigned int eip)
{

  char s[16];

  // Display the instruction pointer that caused the page fault
  hex_convertor(eip, s);
  printk("Process generates a PAGE FAULT exception at EIP: 0x");
  printk(s);
  printc('\n');
  
  // Display the page fault error code
  hex_convertor(error, s);
  printk("Error Code: 0x");
  printk(s);
  printc('\n');

  while(1);  
}


void setIdt()
{
  /* Program interrups/exception service routines */
  idtR.base  = (DWord)idt;
  idtR.limit = IDT_ENTRIES * sizeof(Gate) - 1;
  
  set_handlers();
  
  /* ADD INITIALIZATION CODE FOR INTERRUPT VECTOR */
  setInterruptHandler(33, keyboard_handler, 0);
  setInterruptHandler(32, clock_handler, 0);
  setInterruptHandler(14, my_page_fault_handler, 0);
  
  // 0x80 default IDT entry for SystemCalls
  // Privilege Level 3, user mode
  setTrapHandler(0x80, system_call_handler, 3);

  // Write MSR registers
  writeMSR(0x174, __KERNEL_CS); // Operating system code segment (CS)
  writeMSR(0x175, INITIAL_ESP); // Operating system stack (ESP) 
  writeMSR(0x176, (DWord)syscall_handler_sysenter);  // Operating system entry point, the handler (EIP)

  set_idt_reg(&idtR);
}