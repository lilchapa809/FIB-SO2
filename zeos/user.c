#include <libc.h>

char buff[24];

int pid;

//Gettime Debugger
void test_gettime()
{
  int t1 = gettime();  
  for (int i = 0; i<10000000; ++i) asm(""); //Loose time
  int t2 = gettime();
  if (t2 > t1) write(1,"Gettime: Succeed\n",17);
}

//Write Debugger
void test_write()
{
  if (write(1,"Write: Succeed\n",15)!=15) perror();
  if (write(2,"error",5)<0) perror(); 

}

//Fast Syscalls & writeMSR Debugger
void test_fsyscalls()
{
  int t1 = gettime_f();
  if (write_f(1,"Write_f: Sysenter Succeed\n",26)!=12);
  if (write_f(2,"Error",5)<0) perror(); 
  int t2 = gettime_f();
  if (t2 <= t1) write_f(1,"Error in gettime_f\n",19);
  if (t2 > t1) write_f(1,"gettime_f: Sysenter Succeed\n",27);
}


int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
  test_gettime();
  test_write();
  test_fsyscalls();
  while(1) {}
}