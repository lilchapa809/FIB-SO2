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

// Defined in interrupt.c
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

int sys_gettime()
{
  return zeos_ticks;
}

int sys_getpid(){
  return current()->PID;
}

int sys_write(int fd, char *buffer, int size)
{
  int ret;

  ret = check_fd(fd, ESCRIPTURA);
  if (ret < 0) return ret;

  if (buffer == 0) return -EFAULT;
  if (size < 0) return -EINVAL;

  if (!access_ok(VERIFY_READ, buffer, (unsigned long) size)) return -EFAULT;

  char kbuffer[256];
  int bytes_left = size;
  int written = 0;

  while (bytes_left > 0)
  {
    int chunk = (bytes_left > 256) ? 256 : bytes_left;

    if (copy_from_user(buffer + written, kbuffer, chunk) < 0)
      return -EFAULT;

    ret = sys_write_console(kbuffer, chunk);
    if (ret < 0) return ret;

    written += ret;
    bytes_left -= ret;
  }

  return written;
}