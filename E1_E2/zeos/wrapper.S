#include<asm.h>

#define SAVE_REG \
    pushl %ebx;\
    pushl %ecx;\
    pushl %edx;

#define RESTORE_REG \
    popl %edx;\
    popl %ecx;\
    popl %ebx;

// ============= FAST SYSCALLS =============
/*********************************************************************
/** Fast Syscall Concepts - Written as "syscallname_f"
** 
** - Put the Syscall ID in %EAX
** - ECX & EDX must be stored in User Stack
** - Return address of the following instruction after SYSENTER 
**    must be stored in User Stack
** - Create fake dynamic link
** - SYSENTER
** - Pop EBP, return_address, and used registers (at least: ECX, EDX)
** - Process result & return
**********************************************************************/


ENTRY(write_f)
    pushl %ebp
    movl %esp,%ebp 
    movl $4, %eax
    SAVE_REG 
    movl 8(%ebp), %edx 
    movl 12(%ebp), %ecx
    movl 16(%ebp), %ebx
    pushl $write_ret_address
    pushl %ebp
    movl %esp,%ebp
    SYSENTER

write_ret_address:
    popl %ebp // -4 bytes (fake dynamic link)
    addl $4, %esp //pop $write_ret_address
    RESTORE_REG
    
    cmpl $0, %eax
    jge write_f_end

write_f_error: 
    negl %eax  
    movl %eax, errno  
    movl $-1, %eax  

write_f_end:
    movl %ebp, %esp 
    popl %ebp  
    ret 

ENTRY(gettime_f)
    pushl %ebp
    movl %esp,%ebp 
    movl $10, %eax
    SAVE_REG 
    pushl $gettime_ret_address
    pushl %ebp
    movl %esp,%ebp
    SYSENTER

gettime_ret_address:
    popl %ebp // -4 bytes (fake dynamic link)
    addl $4, %esp //pop $write_ret_address
    RESTORE_REG

    cmpl $0, %eax 
    jge gettime_f_end
 
gettime_f_error:
    negl %eax 
    movl %eax, errno 
    movl $-1, %eax    

gettime_f_end:
    RESTORE_REG
    movl %ebp, %esp  
    popl %ebp
    ret

ENTRY(getpid_f)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    movl $20, %eax        # System call number for getpid (20)
    
    # ! Follow the System V ABI contract 
    # Save registers that will be used here 
    SAVE_REG        

    # No arguments to pass

    # Store return addres getpid_f_return 
    pushl $getpid_f_return

    # Create fake dynamic link 
    pushl %ebp
    movl %esp, %ebp

    SYSENTER            # Generate FAST system call interrupt

getpid_f_return:
    # Clean up the stack 
    popl %ebp
    addl $4, %esp

    # Restore saved registers 
    RESTORE_REG

    # Check if it returns an error (negative value) 
    cmpl $0, %eax        # Compare result with 0 
    jge getpid_f_end        # If positive or zero, return as is

getpid_f_error:
    # Handle error case 
    negl %eax            # Convert to positive value 
    movl %eax, errno     # Store error code in errno 
    movl $-1, %eax       # Return -1 to indicate error        

getpid_f_end:
    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller

ENTRY(fork_f)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    movl $2, %eax        # System call number for fork (2)
    
    # ! Follow the System V ABI contract 
    # Save registers that will be used here 
    SAVE_REG        

    # No arguments to pass

    # Store return addres fork_return 
    pushl $fork_f_return

    # Create fake dynamic link 
    pushl %ebp
    movl %esp, %ebp

    SYSENTER            # Generate FAST system call interrupt

fork_f_return:
    # Clean up the stack 
    popl %ebp
    addl $4, %esp

    # Restore saved registers 
    RESTORE_REG

    # Check if it returns an error (negative value) 
    cmpl $0, %eax        # Compare result with 0 
    jge fork_f_end        # If positive or zero, return as is

fork_f_error:
    # Handle error case 
    negl %eax            # Convert to positive value 
    movl %eax, errno     # Store error code in errno 
    movl $-1, %eax       # Return -1 to indicate error        

fork_f_end:
    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller


ENTRY(exit_f)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    movl $1, %eax        # System call number for exit (1)
    
    # ! Follow the System V ABI contract 
    # Save registers that will be used here 
    SAVE_REG        

    # Store return addres exit_return 
    pushl $exit_f_return

    # Create fake dynamic link 
    pushl %ebp
    movl %esp, %ebp

    SYSENTER            # Generate FAST system call interrupt

exit_f_return:
    # Clean up the stack 
    popl %ebp
    addl $4, %esp

    # Restore saved registers 
    RESTORE_REG

    # ! No need to check for error as exit never returns

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller

ENTRY(block_f)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    movl $12, %eax        # System call number for block (12)
    
    # ! Follow the System V ABI contract 
    # Save registers that will be used here 
    SAVE_REG        

    # No arguments to pass

    # Store return addres block_return 
    pushl $block_f_return

    # Create fake dynamic link 
    pushl %ebp
    movl %esp, %ebp

    SYSENTER            # Generate FAST system call interrupt

block_f_return:
    # Clean up the stack 
    popl %ebp
    addl $4, %esp

    # Restore saved registers 
    RESTORE_REG

    # ! No need to check for error as block never returns

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller


ENTRY(unblock_f)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    movl $13, %eax        # System call number for unblock (13)
    
    # ! Follow the System V ABI contract 
    # Save registers that will be used here 
    SAVE_REG        

    movl 8(%ebp), %edx   # First param: pid -> edx 

    # Store return addres unblock_f_return 
    pushl $unblock_f_return

    # Create fake dynamic link 
    pushl %ebp
    movl %esp, %ebp

    SYSENTER            # Generate FAST system call interrupt

unblock_f_return:
    # Clean up the stack 
    popl %ebp
    addl $4, %esp

    # Restore saved registers 
    RESTORE_REG

    # Check if it returns an error (negative value) 
    cmpl $0, %eax            # Compare result with 0 
    jge unblock_f_end        # If positive or zero, return as is

unblock_f_error:
    # Handle error case 
    negl %eax            # Convert to positive value 
    movl %eax, errno     # Store error code in errno 
    movl $-1, %eax       # Return -1 to indicate error

unblock_f_end:
    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller


// ================ SYSCALLS ================

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
    movl %eax, errno  
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
    movl %eax, errno // Store error code in errno 
    movl $-1, %eax // Return -1 to indicate error    

gettime_end:
    RESTORE_REG
    movl %ebp, %esp // Restore stack pointer 
    popl %ebp // Restore old ebp 
    ret // Return to caller 

ENTRY(getpid)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    # ! Follow the ABI contract 
    # Save the registers 
    SAVE_REG

    # No arguments to pass 

    movl $20, %eax        # System call number for getpid (20) 
    int $0x80

getpid_end:
    # Restore the registers 
    RESTORE_REG

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller

ENTRY(fork)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    # ! Follow the ABI contract 
    # Save the registers 
    SAVE_REG

    # No arguments to pass 

    movl $2, %eax        # System call number for fork (2) 
    int $0x80

    # Check if it returns an error (negative value) 
    cmpl $0, %eax        # Compare result with 0 
    jge fork_end     # If positive or zero, return as is

fork_error:
    # Handle error case 
    negl %eax            # Convert to positive value 
    movl %eax, errno     # Store error code in errno 
    movl $-1, %eax       # Return -1 to indicate error

fork_end:
    # Restore the registers 
    RESTORE_REG

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller

ENTRY(exit)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    # ! Follow the ABI contract 
    # Save the registers 
    SAVE_REG

    movl $1, %eax        # System call number for exit (1) 
    int $0x80

    # ! No need to check for error as exit never returns

    # Restore the registers 
    RESTORE_REG

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller

ENTRY(block)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    # ! Follow the ABI contract 
    # Save the registers 
    SAVE_REG

    # No arguments to pass 

    movl $12, %eax        # System call number for block (12) 
    int $0x80

    # ! No need to check for error as block 

    # Restore the registers 
    RESTORE_REG

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller


ENTRY(unblock)
    pushl %ebp            # Save old ebp 
    movl %esp, %ebp       # Set up new stack frame 

    # ! Follow the ABI contract 
    # Save the registers 
    SAVE_REG

    movl 8(%ebp), %edx   # First param: pid -> edx 

    movl $13, %eax        # System call number for unblock (13) 
    int $0x80

    # Check if it returns an error (negative value) 
    cmpl $0, %eax        # Compare result with 0 
    jge unblock_end   # If positive or zero, return as is

unblock_error:
    # Handle error case 
    negl %eax            # Convert to positive value 
    movl %eax, errno     # Store error code in errno 
    movl $-1, %eax       # Return -1 to indicate error

unblock_end:
    # Restore the registers 
    RESTORE_REG

    movl %ebp, %esp      # Restore stack pointer 
    popl %ebp            # Restore old ebp 
    ret                  # Return to caller