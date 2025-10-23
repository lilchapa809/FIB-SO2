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

/* Ticks since the system started */
int zeos_ticks = 0;


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



/* Converts an unsigned integer to a hexadecimal string */
  void hex_convertor(unsigned int address, char *s) {
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


  //Keyboard Routine
  void keyboard_routine(){
    //screen size, 80x25 --> 0x4F, 0x18 screen limits
    unsigned char kb_register = inb(0x60); // Read a byte from 0xXX port
    unsigned char is_make = kb_register & 0x80; //kb_register[7] --> mask applied 1000 0000
    unsigned char code = kb_register & 0x7F; //kb_register[6..0] --> mask applied 0111 1111
    if (!is_make){
      char to_print = char_map[code];
      if (to_print != 0) printc_xy(0x00,0x00,to_print);
      
      else {
        //clear screen
        for (unsigned int i = 0; i < 0x4f; ++i){
          for (unsigned int j = 0; j < 0x18; ++j){
          printc_xy(i,j,' ');
        }
      }

      printc_xy(0x00,0x00,'C');
    }
  }
}

//Clock routine
void clock_routine(){
  //Time goes faster than the real one
  ++zeos_ticks;
  zeos_show_clock();
}

//Page Fault Routine (already in libzeos.a)
void my_page_fault_routine(unsigned int error, unsigned int eip){
  
  //__asm__ __volatile__("mov %%cr2, %0" :: "r" (error_address)); //%cr2(Control Register 2)
  //__asm__ __volatile__("movl 0(%%esp), %0" :: "r" (eip));
  
  char s[16];
  hex_convertor(eip, s);
  printk("Process generates a PAGE FAULT exception at EIP: 0x");
  printk(s);
  printc('\n');
  
  hex_convertor(error, s);
  printk("                                     Error Code: 0x");
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
  
    //0x80 default IDT entry for SystemCalls
    //Privilege Level 3, user mode
  setTrapHandler(0x80, system_call_handler, 3);


  set_idt_reg(&idtR);
}