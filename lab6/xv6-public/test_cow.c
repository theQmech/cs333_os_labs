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
  // int a = 1;// b = 1;

  printf(1, "\nStarting test #1: \n=============================\n");
  int n = 1, i = 0;
  printf(1, "Num free pages at start are %d\n", getNumFreePages());

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
      int a = 2;
      printf(1, "Child invoke write %d - var %d\n", getpid(), a);
      printf(1, "Num free pages now are %d\n", getNumFreePages());
      exit();
    }

  }
  int z_id;
  while ((z_id = wait()) != -1){
    printf(1, "[%d] reaped\n", z_id);
  }
  printf(1, "=============================\nTest #1 done\n");


  printf(1, "\nStarting test #2: \n=============================\n");


  printf(1, "Num free pages at start are %d\n", getNumFreePages());

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
      int a = 2;
      printf(1, "Child invoke write %d - var %d\n", getpid(), a);
      printf(1, "Num free pages now are %d\n", getNumFreePages());
      exit();
    }

  }
  int z_id;
  while ((z_id = wait()) != -1){
    printf(1, "[%d] reaped\n", z_id);
  }


  exit();

}