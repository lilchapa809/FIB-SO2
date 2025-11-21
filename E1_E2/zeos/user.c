#include <libc.h>
#include <types.h>

char *buff;


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

//Getpid Debuggers
void test_getpid() 
{
  // Print a newline
  write(1, "\n", 1);

  // Test getpid
  buff = "Testing getpid: PID = ";
  write(1, buff, strlen(buff));
  
  // Print PID (should be 1)
  pid = getpid();
  if (pid == 1)
  {
    buff = "getpid: Succeed\n";
    write(1,buff,strlen(buff));  
    buff = "PID number = ";
    write(1,buff,strlen(buff));
    itoa(pid, buff);
    write(1, buff, strlen(buff));
    write(1,  "\n", 1);
  } 
  else 
  {
    buff = "Error in getpid\n";
    write(1,buff,strlen(buff));
  }

}

//Fork and Block debuggers
int test_fork_and_blocks() 
{
  int pid_test = fork();
  if (pid_test == 0) 
  {
    int pid_test2 = fork();
    if (pid_test2 > 0)
    {
      //PROCESS 2 -> unblock test
      buff = "\nIm the PROCESS: ";  // PROCESS 1
      if (write(1,buff,17) == -1) {perror(); exit();}
      itoa(getpid(),buff);
      if (write(1,buff,2) ==-1) {perror(); exit();}

      while(1)
      {
        if (unblock(pid_test2) == -1) {perror(); exit();}
      }
    }
    else if (pid_test2 == 0)
    {
      //PROCESS pid_test2 -> block test
      buff = "\nIm the PROCESS: ";    // PROCESS 2
      if (write(1,buff,17) == -1) {perror(); exit();}
      itoa(getpid(),buff);
      if (write(1,buff,2) == -1) {perror(); exit();}
      buff = " im going to block!\n";
      if (write(1,buff,21) == -1) {perror(); exit();}
      block();
      buff = "Unlocked! \n";
      if (write(1,buff,12) == -1) {perror(); exit();}
      
      //EXIT -> should not write 
      exit(); 
      buff = "\nI should not be here\n";  
      if (write(1,buff,21) == -1) {perror(); exit();}
    }
    else perror();
  }
  else if (pid_test > 0)
  {
    //PROCESS 1 -> Task1
    buff = "Im the PROCESS: ";
    if (write(1,buff,17) == -1) {perror(); exit();}
    itoa(getpid(),buff);
    if (write(1,buff,2) == -1) {perror(); exit();}
    write(1, "\n", 1);
  }
  else 
  {
    perror(); 
    exit();
  }

  return 1;
}

int __attribute__ ((__section__(".text.main")))
  main(void)
{
  /* Next line, tries to move value 0 to CR3 register. This register is a privileged one, and so it will raise an exception */
  /* __asm__ __volatile__ ("mov %0, %%cr3"::"r" (0) ); */
  
  test_gettime();
  test_write();
  test_fsyscalls();
  test_getpid();

  int test_E2_passed = 0;
  if (test_fork_and_blocks()) 
  {
    buff = "Tests E2 passed! :) \n";
    write(1,buff,strlen(buff));
  }
  else{
    buff = "Tests E2 NOT passed :( \n";
    write(1,buff,strlen(buff));
  }

  while(1) {}
}