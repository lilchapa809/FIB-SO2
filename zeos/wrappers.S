/*
 * wrappers.S - Wrappers for system calls
 *       
 */

/* 
 * Steps to implement a system call wrapper:
 *      - Parameter passing
 *      - Put syscall identifier in eax
 *      - Generate the trap: int 0x80
 *      - Process result
 *      - Return 
 */

#include <asm.h>
#include <segment.h>

/*
 * Macro to implement parameter passing by registers
 *   - The first 6 parameters are passed by registers (edx, ecx, edx, esi, edi, ebp)
 */
#define SAVE_REGISTERS \
      pushl %ebp; \
      pushl %edi; \
      pushl %esi; \
      pushl %ebx; \
      pushl %ecx; \
      pushl %edx; \ 

#define RESTORE_REGISTERS \
      popl %edx; \
      popl %ecx; \
      popl %ebx; \
      popl %esi; \
      popl %edi; \
      popl %ebp; \

ENTRY(gettime)
    pushl %ebp
    movl %esp, %ebp 
    SAVE_REGISTERS
                      //No parameters to pass
    movl $10, %eax    //Syscall identifier for gettime(10)  
    int $0x80         //Generate the trap
                      //Process the result
    cmpl $0, %eax     //Is eax(result) negative?
    jge gettime_fin  
 
  gettime_err:      
    negl %eax         //Make it positive
    movl %eax, errno  //Store the error code in errno
    movl $-1, %eax    //Return -1 to indicate an error 

  gettime_fin:
    RESTORE_REGISTERS
    movl %ebp, %esp   //Return to the caller
    popl %ebp  
    ret  
