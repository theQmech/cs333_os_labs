//test scheduler here
#include "types.h"
#include "stat.h"
#include "user.h"

int
main(void)
{

	printf(1, "Testing starts here. \n");

	int n = 10, i = 0;
	printf(1, "Parent process %d parent %d started with priority %d.\n", getpid(), getppid(), getpriority());
	for (; i < n ; i++) {

		int k = fork() ;

		if (k)
		{
			continue;
		}
		else {
			//sleep(n-i);
			setpriority(i);
			if (i == 0) {
				int j = 0;
				for (; j < 1000000; j++);
			}


			// printf(1, "Process %d parent %d started  with priority %d.\n", getpid(), getppid(), getpriority());
			printf(1, "Process %d parent %d exited   with priority %d.\n", getpid(), getppid(), getpriority());
			exit();
		}

	}
	printf(1, "Parent process %d parent %d exited with priority %d.\n", getpid(), getppid(), getpriority());
	exit();

}