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

  //Generate a page fault by trying to read from an invalid address
  //char* p = 0;
  //*p = 'x';
  
  /* Using GDB 
   * #(gdb) info line user.c:40
   *    Line 40 of "user.c" starts at address 0x41418d <main+397>
   *    and ends at 0x414192 <main+402>.
   *    
   *    Process generates a PAGE FAULT exception
   *      EIP: 0x00414197
   *      Error Code: 0x00000006
   */

  while(1);
}
