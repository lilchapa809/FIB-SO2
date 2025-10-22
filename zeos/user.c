#include <libc.h>

char buff[24];

int pid;

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */

  //GETTIME DEBUGGER
  //The goal is to generate the page fault before a while(1)
  int t1 = gettime(); //t1 != 0 neither 1 (logic??)
  if (t1 == 1) while(1);
  
  //loose time
  int tmp = 0;
  int tmp2 = 0;
  for (int i = 0; i<10000000; ++i)++tmp; 
  for (int i = 0; i<1000000; ++i)++tmp2;
  tmp += tmp2; 
  
  int t2 = gettime();
  if (t1 == t2) while(1); // must be t1!=t2, but THEY'RE THE SAME TT

  char *c = 0;
  *c = 'x';

  while(1) { }
}