#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  write (1,"\n",1);
  write (1, "Starting test gettime...\n", 25);
  int i = gettime();
  itoa (i, buff);
  write (1, "Current time: ", 14);
  write (1, buff, strlen(buff));
  write (1, " ticks\n", 7);

  for (int j = 0; j < 1000000; j++) asm ("");
  write (1, "Finished some work...\n", 22);
  int j = gettime();
  write (1, "Time elapsed: ", 14);
  itoa(j - i, buff);
  write (1, buff, strlen(buff));
  write (1, " ticks\n", 7);
  

  int n = write(1, "WRITE TEST\n", 11);
  if (n < 0) {
    perror("write");
  }
  n = write(2, "WRITE TEST ERROR\n", 18);
  if (n < 0) {
    perror("write");
  }

  pid = getpid();
  write (1, "PID: ", 5);
  itoa(pid, buff);
  write (1, buff, strlen(buff));
  write (1, "\n", 1);


  while(1);
}
