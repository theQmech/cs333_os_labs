#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_SERV_SIZE 64
#define NUM_PROC 64
#define REAP_SLEEP_TIME 0.5
#define PRINT 0				//to be used only for debugging

pid_t *bg_queue;		//array of background processes
pthread_mutex_t lock;	//lock for access of *bg_queue
int over; 				//exit command has been entered

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void sig_handler(int signo) {
	if (signo != SIGINT) return;
	if (PRINT) printf("\nInterrupt recieved...%d \n", getpid());
	int status = 0;
	while ( waitpid(0, &status, 0) > 0){} 
	// wait for all children with same pgid to die
}

void enqueue(pid_t id){
	pthread_mutex_lock(&lock);
	for (int i=0; i<NUM_PROC; ++i){
		if (bg_queue[i] == 0){
			bg_queue[i] = id;
			break;
		}
	}
	pthread_mutex_unlock(&lock);
	return;
}

void dequeue(pid_t id){
	pthread_mutex_lock(&lock);
	for (int i=0; i<NUM_PROC; ++i){
		if (bg_queue[i] == id){
			bg_queue[i] = 0;
			break;
		}
	}
	pthread_mutex_unlock(&lock);
	return;
}

void * reap(void *t){

	//reap all zombie background process
	pid_t pid;
	int zomb;
	int noBG = 1; // 1 iff bg_queue is empty
	while (1){
		noBG = 1;

		for (int i=0; i<NUM_PROC; ++i){
			if (bg_queue[i] == 0) continue;
			noBG = 0;
			if ((zomb = waitpid(bg_queue[i], NULL, WNOHANG)) > 0){
				if (zomb != bg_queue[i]) error("incorrect pid reaped"); //sanity check
				dequeue(bg_queue[i]);
				printf("BG [%d] child reaped\n", zomb);
			}
		}
		// over = 1 => exit command entered in shell
		if (noBG && over) break;
		sleep(REAP_SLEEP_TIME);
	}
	pthread_exit(NULL);
}

char **tokenize(char *line) {
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++) {

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t') {
			token[tokenIndex] = '\0';
			if (tokenIndex != 0) {
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else {
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

void runlinuxcmd(char **tokens) {
	if (execvp(tokens[0], tokens) == -1) {
		printf("Couldn't execute command %s\n", tokens[0]);
		exit(0);
	}
}

void get_seq(char **usr_cmd, int narg, char *serv_name, char *serv_pno) {

	int status = 0;
	int fileno = 1;
	for (; fileno < narg ; fileno++){

		int child = 0;
		if ( (child = fork()) != 0) {
			waitpid(child, &status, 0);
			// store error of wait in status and use it later
		}
		else {
			char **tokens = (char **)malloc(6 * sizeof(char *));
			for (int i = 0; i < 5; ++i) {
				tokens[i] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
			}
			strcpy(tokens[0], "./get-one-file-sig");
			strcpy(tokens[1], usr_cmd[fileno]);
			strcpy(tokens[2], serv_name);
			strcpy(tokens[3], serv_pno);
			strcpy(tokens[4], "nodisplay");
			tokens[5] = NULL;
			runlinuxcmd(tokens);
			//never return
		}
			
		if (PRINT) printf("%d  %d\n", fileno, status);
		if (status > 0) break; //status = 0 iff child exited normally
		// if not then SIGINT was recieved, so don't continue
	}
}

void get_pl(char **usr_cmd, int narg, char *serv_name, char *serv_pno) {

	int child = 0;
	if ( (child = fork()) != 0) {
		waitpid(child, NULL, 0);
	}
	else {

		int child2 = 0, status = 0;
		for (int j = 0; j < narg - 1 ; j++) {

			if ((child2 = fork()) != 0 ) {
				//waitpid(child2, NULL, 0);
			}
			else {

				char ** tokens = (char**)malloc(6 * sizeof(char *));
				for (int i = 0; i < 5; ++i) {
					tokens[i] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
				}

				strcpy(tokens[0], "./get-one-file-sig");
				strcpy(tokens[1], usr_cmd[1 + j]);
				strcpy(tokens[2], serv_name);
				strcpy(tokens[3], serv_pno);
				strcpy(tokens[4], "nodisplay");
				tokens[5] = NULL;

				printf("child i %d\n", j);
				runlinuxcmd(tokens);

				for (int i = 0; i < 5; ++i) free(tokens[i]);
				free(tokens);
			}

		}

		while ( wait(&status) > 0) ; // wait for all children to die
		//printf("All files downloaded. \n");
		exit(0); // exit after all children die

	}
}

void get_bg(char **usr_cmd, char *serv_name, char *serv_pno){
	// maintain two queues, one that holds all BG processes
	// another maintains all threads that reap the BG process

	char ** tokens = (char**)malloc(6 * sizeof(char *));
	for (int i = 0; i < 5; ++i) {
		tokens[i] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
	}

	strcpy(tokens[0], "./get-one-file-sig");
	strcpy(tokens[1], usr_cmd[1]);
	strcpy(tokens[2], serv_name);
	strcpy(tokens[3], serv_pno);
	strcpy(tokens[4], "nodisplay");
	tokens[5] = NULL;
	
	int child = fork();
	if ( child  != 0){
		enqueue(child);
		setpgid(child, child);
		//enqueue and set pgid separately so that doesn't get SIGINT
	}
	else{
		runlinuxcmd(tokens);
	}

	for (int i = 0; i < 5; ++i) free(tokens[i]);
	free(tokens);
}

void get_file(char **usr_cmd, char *serv_name, char *serv_pno) {

	// assume usr_cmd passed is valid, do required checking in main()

	int child = 0;
	if ((child = fork()) != 0) {
		waitpid(child, NULL, 0);
	}
	else {
		int p[2];
		char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
		for (int i = 0; i < 5; ++i) {
			tokens[i] = (char*)malloc(MAX_TOKEN_SIZE * sizeof(char));
		}
		strcpy(tokens[0], "./get-one-file-sig");
		strcpy(tokens[1], usr_cmd[1]);
		strcpy(tokens[2], serv_name);
		strcpy(tokens[3], serv_pno);
		strcpy(tokens[4], "display");
		tokens[5] = NULL;

		if (usr_cmd[2] != NULL){

			//handle redirect case case
			if (strcmp(usr_cmd[2], ">") == 0) {
				char *buf = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(buf, usr_cmd[3]);
				int redir_fd = open(buf, O_WRONLY);
				if (redir_fd == -1) {
					printf("Couldn't open output file - %s\n", buf);
					exit(0);
				}
				if (dup2(redir_fd, 1) == -1) {
					printf("Error in duplication\n");
					exit(0);
				}
				close(redir_fd);
			}

			// handle pipe case, separately
			// doesn't return
			else if (strcmp(usr_cmd[2], "|") == 0) {
				if (pipe(p) < 0) {
					printf("Pipe opening error");
					exit(0);
				}
				if (fork() == 0) {
					dup2(p[1], 1);
					close(p[0]);
					close(p[1]);
					runlinuxcmd(tokens);
				}
				char **temp;
				if (fork() == 0) {
					dup2(p[0], 0);
					close(p[0]);
					close(p[1]);
					temp = (char **) malloc(MAX_TOKEN_SIZE * sizeof(char *));
					int i=3;
					while(usr_cmd[i] != NULL){
						temp[i-3] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
						strcpy(temp[i-3], usr_cmd[i]);
						++i;
					}
					temp[i] = NULL;
					runlinuxcmd(temp);
				}
				close(p[0]);
				close(p[1]);
				wait(NULL);
				wait(NULL);
				for (int i = 0; i < 5; ++i) free(tokens[i]);
				free(tokens);
				for (int i = 0; i < MAX_NUM_TOKENS; ++i) free(temp[i]);
				free(temp);
				exit(0);
			}

		}

		runlinuxcmd(tokens);

		for (int i = 0; i < MAX_NUM_TOKENS; ++i) free(tokens[i]);
		free(tokens);
	}
}

void main(void) {
	
	// initialize bg_queue with 0
	bg_queue = (pid_t *)malloc(NUM_PROC * sizeof(pid_t));
	for (int i=0; i<NUM_PROC; ++i) bg_queue[i] = 0;

	pthread_t rthread_id; //reap thread
	int t_rc = pthread_create(&rthread_id, NULL, reap, NULL);

	pthread_mutex_init(&lock, NULL);

	signal(SIGINT, sig_handler);

	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i = 0;
	over = 0;
	int serv_added = 0;
	char * serv_name = (char *) malloc(MAX_SERV_SIZE * sizeof(char *));
	char * serv_pno = (char *) malloc(MAX_SERV_SIZE * sizeof(char *));
	bzero(serv_name, MAX_SERV_SIZE);
	bzero(serv_pno, MAX_SERV_SIZE);

	while (! over) {

		printf("Hello>");
		bzero(line, MAX_INPUT_SIZE);
		gets(line);
		if (PRINT) printf("Got command %s\n", line);
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);

		int narg = 0;
		for (i = 0; tokens[i] != NULL; i++) {
			if (PRINT) printf("found token %s\n", tokens[i]);
			++narg;
		}
		if (narg == 0) continue;

		//temporary block
		if (strcmp(tokens[0], "cd") == 0) {
			if (narg != 2) {
				printf("cd accepts only exactly argument\n");
			}
			else if ( chdir(tokens[1]) != 0) {
				printf("Couldn't change directory to %s\n", tokens[1]);
			}
			continue;
		}
		else if (strcmp(tokens[0], "server") == 0) {
			if (narg != 3) {
				printf("usage: server <server-addr> <port_no>\n");
			}
			else {
				bzero(serv_name, MAX_SERV_SIZE);
				strcpy(serv_name, tokens[1]);
				bzero(serv_pno, MAX_SERV_SIZE);
				strcpy(serv_pno, tokens[2]);
				printf("Server updated: \t%s:%s\n", serv_name, serv_pno);
				serv_added = 1;
			}
		}
		else if (strcmp(tokens[0], "exit") == 0) {
			over = 1;
			for (int i=0; i<NUM_PROC; ++i){
				if (bg_queue[i] == 0) continue;
				kill(bg_queue[i], SIGINT);
				if (PRINT) printf("kill signal [%d]\n", bg_queue[i]);
			}
			pthread_join(rthread_id, NULL);
			printf("Shell exiting\n");
		}
		else if (strcmp(tokens[0], "getfl") == 0) {
			if (serv_added) {
				if (narg == 2) {
					get_file(tokens, serv_name, serv_pno);
				}
				else if (narg == 4 && (strcmp(tokens[2], ">") == 0) ) {
					get_file(tokens, serv_name, serv_pno);
				}
				else if (narg >= 4 && (strcmp(tokens[2], "|") == 0) ) {
					get_file(tokens, serv_name, serv_pno);
				}
				else {
					printf("usage: getfl <file_path>\n");
				}
			}
			else {
				printf("Server not specified. Quitting.\n");
			}
		}
		else if (strcmp(tokens[0], "getsq") == 0) {
			if (serv_added) {
				if (narg > 1) {
					get_seq(tokens, narg, serv_name, serv_pno);
				}
				else {
					printf("usage: getfl <file_path1>  <file_path2> \n");
				}
			}
			else {
				printf("Server not specified. Quitting.\n" );
			}
		}
		else if (strcmp(tokens[0], "getpl") == 0) {
			if (serv_added) {
				if (narg > 1) {
					get_pl(tokens, narg, serv_name, serv_pno);
				}
				else {
					printf("usage: getfl <file_path1>  <file_path2> \n");
				}
			}
			else if (narg == 4 && (strcmp(tokens[2], "|") == 0) ) {
				get_file(tokens, serv_name, serv_pno);
			}
			else {
				printf("Server not specified. Quitting.\n" );
			}

		}
		else if (strcmp(tokens[0], "getbg") == 0) {
			if (serv_added) {
				if (narg == 2) {
					get_bg(tokens, serv_name, serv_pno);
				}
				else {
					printf("usage: getbg <file_path1>  \n");
				}
			}
			else {
				printf("Server not specified. Quitting.\n" );
			}

		}
		else {
			int child;
			if ((child = fork()) == 0) {
				runlinuxcmd(tokens);
				//doesn't return;
			}
			waitpid(child, NULL, 0);
		}

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++) {
			free(tokens[i]);
		}
		free(tokens);
	}

	pthread_mutex_destroy(&lock);


}

