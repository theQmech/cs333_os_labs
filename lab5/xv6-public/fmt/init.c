8400 // init: The initial user-level program
8401 
8402 #include "types.h"
8403 #include "stat.h"
8404 #include "user.h"
8405 #include "fcntl.h"
8406 
8407 char *argv[] = { "sh", 0 };
8408 
8409 int
8410 main(void)
8411 {
8412   int pid, wpid;
8413 
8414   if(open("console", O_RDWR) < 0){
8415     mknod("console", 1, 1);
8416     open("console", O_RDWR);
8417   }
8418   dup(0);  // stdout
8419   dup(0);  // stderr
8420 
8421   for(;;){
8422     printf(1, "init: starting sh\n");
8423     pid = fork();
8424     if(pid < 0){
8425       printf(1, "init: fork failed\n");
8426       exit();
8427     }
8428     if(pid == 0){
8429       exec("sh", argv);
8430       printf(1, "init: exec sh failed\n");
8431       exit();
8432     }
8433     while((wpid=wait()) >= 0 && wpid != pid)
8434       printf(1, "zombie!\n");
8435   }
8436 }
8437 
8438 
8439 
8440 
8441 
8442 
8443 
8444 
8445 
8446 
8447 
8448 
8449 
