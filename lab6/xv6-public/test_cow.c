//test scheduler here
#include "types.h"
#include "stat.h"
#include "user.h"
#include "fcntl.h"

int a = 2;
int b[2048];

int
main(void)
{

  printf(1, "Testing starts here. \n");
  printf(1, "Process info format: [<PID> <PPID> <PRIO>] \n");
  // int a = 1;// b = 1;

  printf(1, "\nStarting test #1: \n=============================\n");
  // int n = 1, i = 0;
  printf(1, "Num free pages at start are %d\n", getNumFreePages());

  printf(1, "Processes forked will invoke CoW now \n");

  char msg[] = "here=======\n";

  if (fork()){
    printf(1, "%s", msg);
    // while(1){}
    // printf(1, "%x\n", getPhyAddr(&a));
    sleep(100);
  }
  else{
    // printf(1, "%x\n", getPhyAddr(&a));
    // while(1){}
    // sleep(100);
    exit();
  }

  int z_id;
  while ((z_id = wait()) != -1){
    printf(1, "[%d] reaped\n", z_id);
  }
  printf(1, "=============================\nTest #1 done\n");


  // printf(1, "\nStarting test #2: \n=============================\n");
  // // int n = 1, i = 0;
  // printf(1, "Num free pages at start are %d\n", getNumFreePages());

  // printf(1, "Processes forked will now enter infinite while(1){} loop, CoW won't be invoked, no return,\n press ^C to exit \n");

  // if (fork()){
  //   while(1){}
  //   sleep(1000);
  // }
  // else{
  //   while(1){}
  //   exit();
  // }
  // while ((z_id = wait()) != -1){
  //   printf(1, "[%d] reaped\n", z_id);
  // }
  // printf(1, "=============================\nTest #2 done\n");




  exit();

}