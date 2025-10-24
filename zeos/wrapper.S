#include<asm.h>

#define SAVE_REG \
    pushl %ebx;\
    pushl %ecx;\
    pushl %edx;

#define RESTORE_REG \
    popl %edx;\
    popl %ecx;\
    popl %ebx;

ENTRY(write)
    pushl %ebp
    movl %esp,%ebp 
    SAVE_REG  
    //Write has 3 parameters (fd,*buffer,size)
    movl 8(%ebp), %edx //fd -> 1st parameter
    movl 12(%ebp), %ecx //buffer -> 2nd parameter
    movl 16(%ebp), %ebx //size -> 3rd parameter
    movl $4,%eax // Syscall number for write is 4
    int $0x80

    cmpl $0, %eax
    jge write_end

write_error: 
    negl %eax  
    movl %eax, zeos_errno  
    movl $-1, %eax  

write_end:
    RESTORE_REG
    movl %ebp, %esp 
    popl %ebp  
    ret 

ENTRY(gettime)
    pushl %ebp // Save old ebp 
    movl %esp, %ebp // Set up new stack frame 
    // Save the registers (ABI contract)
    SAVE_REG
    //No parameters to pass
    movl $10, %eax // System call number for gettime (10) 
    int $0x80    
    // Check if it returns an error (negative value) 
    cmpl $0, %eax // Compare result with 0 
    jge gettime_end // If positive or zero, return as is 
 
gettime_error:
    negl %eax // Convert to positive value 
    movl %eax, zeos_errno // Store error code in zeos_errno 
    movl $-1, %eax // Return -1 to indicate error    

gettime_end:
    RESTORE_REG
    movl %ebp, %esp // Restore stack pointer 
    popl %ebp // Restore old ebp 
    ret // Return to caller 