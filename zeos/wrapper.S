#include<asm.h>

#define SAVE_REG \
    pushl %ebx;\
    pushl %ecx;\
    pushl %edx;

#define RESTORE_REG \
    popl %edx;\
    popl %ecx;\
    popl %ebx;


ENTRY(gettime)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    # Follow the ABI contract 
    # Save the registers 
    SAVE_REG

    # No arguments to pass 

    movl $10, %eax        # System call number for gettime (10) 
    int $0x80
    
    # Check if it returns an error (negative value) 
    cmpl $0, %eax        # Compare result with 0 
    jge gettime_end   # If positive or zero, return as is 
 
gettime_error:
    # Handle error case 
    negl %eax            # Convert to positive value 
    movl %eax, errno     # Store error code in errno 
    movl $-1, %eax       # Return -1 to indicate error    

gettime_end:
    # Restore the registers 
    RESTORE_REG

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller 