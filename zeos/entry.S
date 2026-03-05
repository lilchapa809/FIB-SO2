/*
 * entry.S - Entry point to system mode from user mode
 */

#include <asm.h>
#include <segment.h>
#include "errno.h"

/**************************************************/
/**** Save & Restore ******************************/
/**                                              **/
/** When we change to privilege level 0 (kernel) **/
/** (through an interrupt, a system call, an     **/
/** exception ...) we must save the state of the **/
/** currently running task (save).               **/
/**                                              **/
/** Stack layout in 'systemCall':                **/
/**                                              **/
/**   0(%esp) - %edx    \                        **/
/**   4(%esp) - %ecx     |                       **/
/**   8(%esp) - %ebx     |                       **/
/**   C(%esp) - %esi     | Register saved        **/
/**  10(%esp) - %edi     |  by 'save'            **/
/**  14(%esp) - %ebp     |                       **/
/**  18(%esp) - %eax     |                       **/
/**  1C(%esp) - %ds      |                       **/
/**  20(%esp) - %es      |                       **/
/**  24(%esp) - %fs      |                       **/
/**  28(%esp) - %gs     /                        **/
/**  2C(%esp) - %eip    \                        **/
/**  30(%esp) - %cs      |                       **/
/**  34(%esp) - %eflags  |  Return context saved **/
/**  38(%esp) - %oldesp  |   by the processor.   **/
/**  3C(%esp) - %oldss  /                        **/
/**                                              **/
/**************************************************/

#define SAVE_ALL \
      pushl %gs; \
      pushl %fs; \
      pushl %es; \
      pushl %ds; \
      pushl %eax; \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \
      movl $__KERNEL_DS, %edx;    \
      movl %edx, %ds;           \
      movl %edx, %es

//Pop the registers added in the System Stack in reverse order
#define RESTORE_ALL \
      popl %edx; \
      popl %ecx; \
      popl %ebx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \
      popl %eax; \
      popl %ds; \
      popl %es; \
      popl %fs; \
      popl %gs;

// End Of Interruption 
#define EOI             \
      movb $0x20, %al ; \
      outb %al, $0x20;

//Syscall Handler (int 0x80)
ENTRY(system_call_handler)        // Assembly header
      SAVE_ALL                    // Save the current context
      cmpl $0, %EAX               // Is syscall number negative?
      jl err                      // If it is, jump to return an error
      cmpl $MAX_SYSCALL, %EAX     // Is syscall greater than MAX_SYSCALL?
      jg err                      // If it is, jump to return an error

      // Prepare up to 3 integer arguments for the C syscall:
      //   arg1 = fd     (user %ebx, saved at 8(%ebx_base))
      //   arg2 = buffer (user %ecx, saved at 4(%ebx_base))
      //   arg3 = size   (user %edx, saved at 0(%ebx_base))
      movl %esp, %ebx             // Base of saved registers
      movl 0x00(%ebx), %edx       // size
      pushl %edx
      movl 0x04(%ebx), %edx       // buffer
      pushl %edx
      movl 0x08(%ebx), %edx       // fd
      pushl %edx

      call *sys_call_table(, %EAX, 0x04) // Call the corresponding service routine
      addl $12, %esp             // Clean arguments from stack
      jmp fin                    // Finish
  err:
      movl $-ENOSYS, %EAX        // Move to EAX the ENOSYS error
  fin:
      movl %EAX, 0x18(%esp)      // Change the EAX value in the stack
      RESTORE_ALL                // Restore the context
      iret

// Fast syscall handler (sysenter)
ENTRY(syscall_handler_sysenter)
      push $__USER_DS          // User data segment
      push %EBP                // User stack address (EBP)
      pushfl
      push $__USER_CS          // User code segment
      push 4(%EBP)             // User return address (after sysenter)
      SAVE_ALL                 // Save full context and switch to kernel segments
      cmpl $0, %EAX            // Is syscall number negative?
      jl sysenter_err          // If it is, jump to return an error
      cmpl $MAX_SYSCALL, %EAX  // Is syscall greater than MAX_SYSCALL?
      jg sysenter_err          // If it is, jump to return an error

      // Prepare up to 3 integer arguments for the C syscall (same layout as int 0x80)
      //   arg1 = fd     (user %ebx, saved at 8(%ebx_base))
      //   arg2 = buffer (user %ecx, saved at 4(%ebx_base))
      //   arg3 = size   (user %edx, saved at 0(%ebx_base))
      movl %esp, %ebx             // Base of saved registers
      movl 0x00(%ebx), %edx       // size
      pushl %edx
      movl 0x04(%ebx), %edx       // buffer
      pushl %edx
      movl 0x08(%ebx), %edx       // fd
      pushl %edx

      call *sys_call_table(, %EAX, 0x04) // Call the corresponding service routine
      addl $12, %esp             // Clean arguments from stack
      jmp sysenter_fin
  sysenter_err:
      movl $-ENOSYS, %EAX      // Move to EAX the ENOSYS error
  sysenter_fin:
      movl %EAX, 0x18(%ESP)    // Change the EAX value in the saved context
      RESTORE_ALL              // Restore registers (including user EBP)
      movl (%ESP), %EDX        // User return address
      movl 12(%ESP), %ECX      // User stack address (user EBP)
      sti                      // Enable interrupts again
      sysexit                  // Return to user mode

//Clock Handler
ENTRY(clock_handler)          // Assembly header
      SAVE_ALL                // Save the context
      EOI                     // End of interruption we notify the system that we received          
                              // and treated the interruption, so we can receive new interruptions
      call clock_routine;     // Call the clock routine in C --> interrupt.c
      RESTORE_ALL             // Restore the context
      iret;                   // Return from the interruption to user mode

//Keyboard Handler
ENTRY(keyboard_handler)       // Assembly header
      SAVE_ALL                // Save the context
      EOI                     // End of interruption we notify the system that we received          
                              // and treated the interruption, so we can receive new interruptions
      call keyboard_routine;     // Call the keyboard routine in C --> interrupt.c
      RESTORE_ALL             // Restore the context
      iret;                   // Return from the interruption to user mode
