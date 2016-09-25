//test scheduler here
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int
main(void)
{

  printf(1, "Testing starts here. \n");
  printf(1, "Process info format: [<PID> <PPID> <PRIO>] \n");

  {
  printf(1, "\nStarting test #1: \n=============================\n");
  setprio(100);
  int n = 10, i = 0;
  printf(1, "[%d, %d, %d] started\n", getpid(), getppid(), getprio());

  for (; i < n ; i++) {

    int k = fork();
    if (k == -1){
      printf(2, "fork failed\n");
    }

    if (k){
      //parent
      continue;
    }
    else{
      //child
      setprio(i+5);
      printf(1, "[%d, %d, %d] starting\n", getpid(), getppid(), getprio());

      int j = 0;
      for (; j < 1000000; j++){}

      printf(1, "[%d, %d, %d] exiting\n", getpid(), getppid(), getprio());

      exit();
    }

  }
  int z_id;
  while ((z_id = wait()) != -1){
    printf(1, "[%d] reaped\n", z_id);
  }
  printf(1, "=============================\nTest #1 done\n");
  }

  {
  printf(1, "\nStarting test #2: \n=============================\n");
  setprio(100);
  int n = 10, i = 0;
  printf(1, "[%d, %d, %d] started\n", getpid(), getppid(), getprio());

  for (; i < n ; i++) {

    int k = fork();
    if (k == -1){
      printf(2, "fork failed\n");
    }

    if (k){
      //parent
      continue;
    }
    else{
      //child
      setprio(i+5);
      printf(1, "[%d, %d, %d] starting\n", getpid(), getppid(), getprio());

      int fd1, fd2;
      if((fd1 = open("dump", O_WRONLY|O_CREATE)) < 0){
        printf(2, "open %s failed\n", "dump");
        exit();
      }
      if((fd2 = open("dump", O_RDONLY|O_CREATE)) < 0){
        printf(2, "open %s failed\n", "dump");
        exit();
      }


      int j = 0;
      int temp = 100;
      char buf[temp+1];
      for (; j < 100; j++){
        temp = read(fd2, buf, temp);
        printf(fd1, "%d\n", j);
      }

      printf(1, "[%d, %d, %d] exiting\n", getpid(), getppid(), getprio());

      close(fd1);
      close(fd2);
      exit();
    }

  }
  int z_id;
  while ((z_id = wait()) != -1){
    printf(1, "[%d] reaped\n", z_id);
  }
  printf(1, "=============================\nTest #2 done\n");
  }
  printf(1, "[%d, %d, %d] exiting\n", getpid(), getppid(), getprio());


  exit();

}