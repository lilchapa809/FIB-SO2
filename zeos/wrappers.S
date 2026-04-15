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

//Write syscall wrapper using sysenter
ENTRY(write)
    pushl %ebp
    movl %esp, %ebp

    //SAVE_REGISTERS
    pushl %ebx
    pushl %esi
    pushl %edi
    pushl %ecx
    pushl %edx

    movl 8(%ebp), %edx     // fd -> edx
    movl 12(%ebp), %ecx    // buffer -> ecx
    movl 16(%ebp), %ebx    // size -> ebx
    movl $4, %eax          // Syscall identifier for write(4)

    // Store return address and create fake dynamic link
    leal write_ret, %esi
    pushl %esi             // User return EIP (after sysenter)
    pushl %ebp             // User stack address (current EBP)
    movl %esp, %ebp        // EBP points to fake frame

    // Enter fast system call
    sysenter

write_ret:
    // Deshacer el fake frame y restaurar el marco del llamador
    addl $8, %esp           // quitar (old_ebp_fake, write_ret) del tope

    //RESTORE_REGISTERS
    popl %edx
    popl %ecx
    popl %edi
    popl %esi
    popl %ebx

    popl %ebp

    // Process the result
    cmpl $0, %eax          // Is eax(result) negative?
    jge write_fin

    negl %eax              // Make it positive
    movl %eax, errno       // Store the error code in errno
    movl $-1, %eax         // Return -1 to indicate an error

write_fin:
    ret


ENTRY(getpid)
    pushl %ebp
    movl %esp, %ebp

    SAVE_REGISTERS

    movl $20, %eax
    int $0x80

    cmpl $0, %eax
    jge getpid_fin

getpid_error:
    negl %eax
    movl %eax, errno
    movl $-1, %eax

getpid_fin:
    RESTORE_REGISTERS
    movl %ebp, %esp
    popl %ebp
    ret
    
