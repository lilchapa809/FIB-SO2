#include <libc.h>

char buff[24];

int pid;

//Gettime Debugger
void test_gettime(){
  //The goal is to generate the page fault before a while(1)
  int t1 = gettime();
  if (t1 == 0) while(1);

  //Loose time
  for (int i = 0; i<10000000; ++i) asm(""); 

  int t2 = gettime();
  if (t2 == t1) /*Should be false*/ while(1);
  char *c = 0;
  *c = 'x';
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
  test_gettime();
  while(1) {}
}