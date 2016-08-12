#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

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


void  main(void)
{
	char  line[MAX_INPUT_SIZE];			
	char  **tokens;			  
	int i;

	while (1) {		   
	   
		printf("Hello>");	 
		bzero(line, MAX_INPUT_SIZE);
		gets(line);			 
		printf("Got command %s\n", line);
		line[strlen(line)] = '\n'; //terminate with new line
		tokens = tokenize(line);
   
		//do whatever you want with the commands, here we just print them

		int n = 0;
		for(i=0;tokens[i]!=NULL;i++){
			printf("found token %s\n", tokens[i]);
			++n;
		}
	   
		//temporary block
		char cdir[] = "cd";
		if (strcmp(tokens[0], cdir) == 0){
			printf("here\n");
			if (n != 2){
				printf("cd accepts only one argument\n");
			}
			else if ( chdir(tokens[1]) != 0){
				printf("Couldn't change directory to %s\n", tokens[1]);
			}
		}

		//another temporary block
		char list[] = "ls\0";
		if (strcmp(tokens[0], list) == 0){
			if (n != 1){
				printf("ls accepts only one argument\n");
			}
			int child = fork();
			if (child != 0){
				if (wait() != child){
					printf("tera LoL ho gaya\n");
				}
			}
			else{
				execl("/bin/ls", "ls", "-l", (char *)0);
			}
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	 

}

				
