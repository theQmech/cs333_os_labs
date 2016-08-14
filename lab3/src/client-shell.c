#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_SERV_SIZE 64
#define PRINT 1				//to be used only for debugging

//bohot fart hain, updae karna hain isko
//should ideally send SIGINT to all foreground children
void sig_handler(int signo){
	if (signo == SIGINT){
		printf("\ninterrrupt recieved... exitting\n");
	}
	exit(15);
}

char **tokenize(char *line){
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for(i =0; i < strlen(line); i++){

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t'){
			token[tokenIndex] = '\0';
			if (tokenIndex != 0){
				tokens[tokenNo] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
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

void runlinuxcmd(char **tokens){
	if (execvp(tokens[0], tokens) == -1){
		printf("Couldn't execute command %s\n", tokens[0]);
		exit(0);
	}
	return;
}

void get_file(char **usr_cmd, char *serv_name, char *serv_pno){

	// assumer usr_cmd passed is valid, do required checking in main()

	int child = 0;
	if ((child = fork()) != 0){
		waitpid(child, NULL, 0);
	}
	else{
		int p[2];
		char **tokens = (char **)malloc(6 * sizeof(char *));
		for (int i=0; i<5; ++i){
			tokens[i] = (char*)malloc(MAX_TOKEN_SIZE*sizeof(char));
		}
		strcpy(tokens[0], "./get-one-file-sig");
		strcpy(tokens[1], usr_cmd[1]);
		strcpy(tokens[2], serv_name);
		strcpy(tokens[3], serv_pno);	
		strcpy(tokens[4], "display");	
		tokens[5] = NULL;

		if (usr_cmd[2] != NULL)
		if (strcmp(usr_cmd[2], ">") == 0){
			char *buf = (char *)malloc(MAX_TOKEN_SIZE*sizeof(char));
			strcpy(buf, usr_cmd[3]);
			int redir_fd = open(buf, O_WRONLY);
			if (redir_fd == -1){
				printf("Couldn't open output file - %s\n", buf);
				exit(0);
			}
			if (dup2(redir_fd, 1) == -1){
				printf("Error in duplication\n");
				exit(0);
			}
			close(redir_fd);
		}
		else if (strcmp(usr_cmd[2], "|") == 0){
			if (pipe(p) < 0){
				printf("Pipe opening error");
				exit(0);
			}
			if (fork() == 0){
				dup2(p[1], 1);
				close(p[0]);
				close(p[1]);
				runlinuxcmd(tokens);
			}
			char **temp;
			if (fork() == 0){
				dup2(p[0], 0);
				close(p[0]);
				close(p[1]);
				temp = (char **) malloc(2*sizeof(char *));
				temp[0] = (char *)malloc(MAX_TOKEN_SIZE*sizeof(char));
				strcpy(temp[0], usr_cmd[3]);
				temp[1] = NULL;
				runlinuxcmd(temp);
			}
			close(p[0]);
			close(p[1]);
			wait(NULL);
			wait(NULL);
			for (int i=0; i<5; ++i) free(tokens[i]);
			free(tokens);
			for (int i=0; i<2; ++i) free(temp[i]);
			free(temp);
			exit(0);
		}

		runlinuxcmd(tokens);

		for (int i=0; i<5; ++i) free(tokens[i]);
		free(tokens);
	}
}

void main(void){

	signal(SIGINT, sig_handler);

	char line[MAX_INPUT_SIZE];			
	char **tokens;			  
	int i, over = 0;
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
   
		//do whatever you want with the commands, here we just print them

		int narg = 0;
		for(i=0;tokens[i]!=NULL;i++){
			if (PRINT) printf("found token %s\n", tokens[i]);
			++narg;
		}
		if (narg == 0) continue;
	   
		//temporary block
		if (strcmp(tokens[0], "cd") == 0){
			if (narg != 2){
				printf("cd accepts only exactly argument\n");
			}
			else if ( chdir(tokens[1]) != 0){
				printf("Couldn't change directory to %s\n", tokens[1]);
			}
			continue;
		}
		else if (strcmp(tokens[0], "server") == 0){
			if (narg != 3) {
				printf("usage: server <server-addr> <port_no>\n");
			}
			else{
				bzero(serv_name, MAX_SERV_SIZE);
				strcpy(serv_name, tokens[1]);
				bzero(serv_pno, MAX_SERV_SIZE);
				strcpy(serv_pno, tokens[2]);
				printf("Server updated: \t%s:%s\n", serv_name, serv_pno);
			}
		}
		else if (strcmp(tokens[0], "exit") == 0){
			printf("Shell exiting\n");
			over = 1;
		}
		else if(strcmp(tokens[0], "getfl") == 0){
			if (narg == 2){
				get_file(tokens, serv_name, serv_pno);
			}	
			else if (narg == 4 && (strcmp(tokens[2], ">")==0) ){
				get_file(tokens, serv_name, serv_pno);
			}
			else if (narg == 4 && (strcmp(tokens[2], "|")==0) ){
				get_file(tokens, serv_name, serv_pno);
			}
			else {
				printf("usage: getfl <file_path>\n");
			}
		}
		else{
			int child;
			if ((child = fork()) == 0){
				runlinuxcmd(tokens);
				//doesn't return;
			}
			waitpid(child, NULL, 0);
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	 

}

