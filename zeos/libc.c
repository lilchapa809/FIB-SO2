/*
 * libc.c 
 */

#include <libc.h>

#include <types.h>

#include <errno.h>

int errno;

void itoa(int a, char *b)
{
  int i, i1;
  char c;
  
  if (a==0) { b[0]='0'; b[1]=0; return ;}
  
  i=0;
  while (a>0)
  {
    b[i]=(a%10)+'0';
    a=a/10;
    i++;
  }
  
  for (i1=0; i1<i/2; i1++)
  {
    c=b[i1];
    b[i1]=b[i-i1-1];
    b[i-i1-1]=c;
  }
  b[i]=0;
}

int strlen(char *a)
{
  int i;
  
  i=0;
  
  while (a[i]!=0) i++;
  
  return i;
}

void perror(const char *s)
{
  char *msg;

  switch (errno)
  {
    case EBADF:  msg = "Bad file number"; break;
    case EACCES: msg = "Permission denied"; break;
    case EFAULT: msg = "Bad address"; break;
    case EINVAL: msg = "Invalid argument"; break;
    case ENOSYS: msg = "Function not implemented"; break;
    default:     msg = "Unknown error"; break;
  }

  if (s && s[0] != 0)
  {
    write(1, (char*)s, strlen((char*)s));
    write(1, ": ", 2);
  }

  write(1, msg, strlen(msg));
  write(1, "\n", 1);
}



