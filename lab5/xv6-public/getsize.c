// A simple program which just prints something on screen

#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{
 printf(1,"Here is ppid of process %d \n", getppid() );
 exit();
}