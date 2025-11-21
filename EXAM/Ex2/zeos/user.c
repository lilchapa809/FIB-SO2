#include <libc.h>

char buff[24];

int pid;

void test_get_time()
{
  int t1, t2;
  
  t1 = gettime();
  itoa(t1, buff);
  write(1, "\nStart time in ticks: ", 21);
  write(1, buff, strlen(buff));
  write(1, "\n", 1);
  
  for (int i = 0; i < 1000000; i++); asm("");
  t2 = gettime();
  itoa(t2, buff);
  write(1, "End time in ticks: ", 18);
  write(1, buff, strlen(buff));
  write(1, "\n", 1);

  if (t2 > t1) {
    itoa(t2 - t1, buff);
    write(1, "Time elapsed in ticks: ", 23);
    write(1, buff, strlen(buff));
    write(1, "\n", 1);
  } else {
    write(1, "Error in time measurement\n", 26);
  }
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
    /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
     /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
  test_get_time();
  while(1) { }
}
