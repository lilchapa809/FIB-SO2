# 0 "wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrapper.S"
# 1 "include/asm.h" 1
# 2 "wrapper.S" 2
# 14 "wrapper.S"
.globl gettime; .type gettime, @function; .align 0; gettime:
    pushl %ebp # Save old ebp
    movl %esp, %ebp # Set up new stack frame

    # Save the registers (ABI contract)
    pushl %ebx; pushl %ecx; pushl %edx;

    # No arguments to pass

    movl $10, %eax # System call number for gettime (10)
    int $0x80

    # Check if it returns an error (negative value)
    cmpl $0, %eax # Compare result with 0
    jge gettime_end # If positive or zero, return as is

gettime_error:
    # Handle error case
    negl %eax # Convert to positive value
    movl %eax, errno # Store error code in errno
    movl $-1, %eax # Return -1 to indicate error

gettime_end:
    # Restore the registers
    popl %edx; popl %ecx; popl %ebx;

    movl %ebp, %esp # Restore stack pointer
    popl %ebp # Restore old ebp
    ret # Return to caller
