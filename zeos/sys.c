/*
 * sys.c - Syscalls implementation
 */
#include <devices.h>

#include <utils.h>

#include <io.h>

#include <mm.h>

#include <mm_address.h>

#include <sched.h>

#include "errno.h"

#define LECTURA 0
#define ESCRIPTURA 1

// Buffer 
#define BUFFER_SIZE 256
char dest_buffer[BUFFER_SIZE];

/* Referenced in interrupt.c */
extern int zeos_ticks;

int check_fd(int fd, int permissions)
{
  if (fd!=1) return -9; /*EBADF*/
  if (permissions!=ESCRIPTURA) return -13; /*EACCES*/
  return 0;
}

int sys_ni_syscall()
{
	return -38; /*ENOSYS*/
}

int sys_getpid()
{
	return current()->PID;
}

int sys_fork()
{
  int PID=-1;

  // creates the child process
  
  return PID;
}

void sys_exit()
{  
}

/**
 * @brief System call service routine for gettime()
 * 
 * Returns the number of clock ticks elapsed since system boot.
 * Each tick corresponds to one clock interrupt, which occurs
 * at regular intervals determined by the system timer.
 * 
 * This system call allows user programs to measure time intervals
 * and implement timing-related functionality.
 * 
 * @return int - Current value of the zeos_ticks global counter
 * 
 * System call number: 10
 * Wrapper function: gettime() in user space
 */
int sys_gettime()
{
  return zeos_ticks;
}

/**
 * @brief System call service routine for write()
 * 
 * Implements the write system call that allows user programs to output
 * data to supported devices. Currently only supports writing to stdout (fd=1).
 * 
 * @param fd File descriptor (must be 1 for stdout in current implementation)
 * @param buffer Pointer to the user-space buffer containing data to write
 * @param size Number of bytes to write from the buffer
 * 
 * @return int On success, returns the number of bytes written. On error, returns:
 *            -EBADF (9) for invalid file descriptor
 *            -EACCES (13) for incorrect permissions  
 *            -EFAULT (14) for invalid buffer address
 *            -EINVAL (22) for invalid size parameter
 * 
 * System call number: 4
 * Wrapper function: write() in user space
 * 
 * @note Currently only supports writing to console (stdout, 1).
 */
int sys_write(int fd, char *buffer, int size)
{
  //Parameter Check
  int error_fd = check_fd(fd,ESCRIPTURA);
  if (error_fd < 0) {
    return error_fd;
  }
  if(buffer == NULL) return -14; //EFAULT
  if (size < 0) return -22; //EINVAL
  
  int copy_error;
  int bytes_left = size; //All bytes remaining to be written
  int bytes_written = 0; //No bytes written yet
  int buffer_offset = size - bytes_left; //Initialized at 0
  //We have to limit the size of the destination of copy_from_user
  while (bytes_left > BUFFER_SIZE)
  {
    copy_error = copy_from_user(buffer + buffer_offset, dest_buffer, BUFFER_SIZE);
    if (copy_error < 0) return -14; //EFAULT

    bytes_written += sys_write_console(dest_buffer,BUFFER_SIZE);
    bytes_left -= BUFFER_SIZE;
    buffer_offset += BUFFER_SIZE;
  }

  if (bytes_left > 0) 
  {
    copy_error = copy_from_user(buffer + buffer_offset, dest_buffer, bytes_left);
    if (copy_error < 0) return -14; //EFAULT
    bytes_written += sys_write_console(buffer,bytes_left);
  }

  return bytes_written;
}