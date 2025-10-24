#include <asm.h>

/*
 * This function writes a value to a Model Specific Register (MSR).
 * Parameters:
 *   - MSR_number 0x17[4,5,6]: The MSR number to write to (first parameter on the stack).
 *   - value: The value to write to the MSR, we use specified MACROS defined at ZeOs PDF.
 */

ENTRY(writeMSR)
    pushl %ebp           
    movl %esp, %ebp      

    movl 8(%ebp), %ecx  // MSR_number -> %ECX
    movl 12(%ebp), %eax // Value to write -> %EAX
    xor %edx, %edx // Set %EDX to 0
    wrmsr //MSR[ECX] = EDX:EAX

    movl %ebp, %esp      
    popl %ebp            
    ret                  