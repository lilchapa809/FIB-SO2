# 0 "wrapper.S"
# 0 "<built-in>"
# 0 "<command-line>"
# 1 "/usr/include/stdc-predef.h" 1 3 4
# 0 "<command-line>" 2
# 1 "wrapper.S"
# 1 "include/asm.h" 1
# 2 "wrapper.S" 2
# 13 "wrapper.S"
.globl write; .type write, @function; .align 0; write:
    pushl %ebp
    movl %esp,%ebp
    pushl %ebx; pushl %ecx; pushl %edx;

    movl 8(%ebp), %edx
    movl 12(%ebp), %ecx
    movl 16(%ebp), %ebx
    movl $4,%eax
    int $0x80

    cmpl $0, %eax
    jge write_end

write_error:
    negl %eax
    movl %eax, zeos_errno
    movl $-1, %eax

write_end:
    popl %edx; popl %ecx; popl %ebx;
    movl %ebp, %esp
    popl %ebp
    ret

.globl gettime; .type gettime, @function; .align 0; gettime:
    pushl %ebp
    movl %esp, %ebp

    pushl %ebx; pushl %ecx; pushl %edx;

    movl $10, %eax
    int $0x80

    cmpl $0, %eax
    jge gettime_end

gettime_error:
    negl %eax
    movl %eax, zeos_errno
    movl $-1, %eax

gettime_end:
    popl %edx; popl %ecx; popl %ebx;
    movl %ebp, %esp
    popl %ebp
    ret
