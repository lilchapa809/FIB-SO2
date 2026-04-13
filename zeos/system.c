/*
 * system.c - 
 */

#include <segment.h>
#include <types.h>
#include <interrupt.h>
#include <hardware.h>
#include <system.h>
#include <sched.h>
#include <mm.h>
#include <io.h>
#include <utils.h>
#include <zeos_mm.h> /* TO BE DELETED WHEN ADDED THE PROCESS MANAGEMENT CODE TO BECOME MULTIPROCESS */

#include <errno.h>

#define SYSENTER_CS_MSR  0x174
#define SYSENTER_ESP_MSR 0x175
#define SYSENTER_EIP_MSR 0x176

int (*usr_main)(void) = (void *) (PAG_LOG_INIT_CODE*PAGE_SIZE);
unsigned int *p_sys_size = (unsigned int *) KERNEL_START;
unsigned int *p_usr_size = (unsigned int *) KERNEL_START+1;
unsigned int *p_rdtr = (unsigned int *) KERNEL_START+2;


/*
 *   Main entry point to ZEOS Operating System
 */
int __attribute__((__section__(".text.main")))
  main(void)
{

  set_eflags();

  /* Define the kernel segment registers  and a stack to execute this 'main' code */
  // It is necessary to use the address of a global static array for the stack,
  // (instead of a variable content with the same address) because the compiler
  // will know its final memory location. Otherwise it will try to use the
  // 'ds' register to access the variable's address... but we are not ready for that yet
  // (we are still in real mode).
  set_seg_regs(__KERNEL_DS, __KERNEL_DS, INITIAL_ESP); // using a global array as the stack

  /*** DO *NOT* ADD ANY CODE IN THIS ROUTINE BEFORE THIS POINT ***/

  printk("Kernel Loaded!    ");


  /* Initialize hardware data */
  setGdt(); /* Definicio de la taula de segments de memoria */
  setIdt(); /* Definicio del vector de interrupcions */
  setTSS(); /* Definicio de la TSS */

  /* Initialize fast system calls (sysenter/sysexit) */
  writeMSR(SYSENTER_CS_MSR, __KERNEL_CS);
  writeMSR(SYSENTER_ESP_MSR, INITIAL_ESP);
  writeMSR(SYSENTER_EIP_MSR, (DWord) syscall_handler_sysenter);

  /* Initialize Memory */
  init_mm();

  /* Multitasking version: init_task1() creates the initial process address space */

  /* Initialize Scheduling */
  init_sched();

  /* Initialize idle task  data */
  init_idle();
  /* Initialize task 1 data */
  init_task1();

  /* Move user code/data now (after the page table initialization) */
  copy_data((void *) KERNEL_START + *p_sys_size, (void*)L_USER_START, *p_usr_size);

  zeos_ticks = 0;

  clear_screen(); //Function added by user to clear first bochs messages

  //Bochs Personalization
  printk(" <-- Keyborad Routine Key\n");
  printk("\n---------------------------------\n");
  printk("SO2 - ZeOs Project\n");
  printk("Marcel Sanz & Gerard Chaparro\n");
  printk("---------------------------------\n\n");
  printk("Entering user mode...");
  enable_int();
  /*
   * We return from a 'theorical' call to a 'call gate' to reduce our privileges
   * and going to execute 'magically' at 'usr_main'...
   */
  return_gate(__USER_DS, __USER_DS, USER_ESP, __USER_CS, (DWord) usr_main);

  /* The execution never arrives to this point */
  return 0;
}


