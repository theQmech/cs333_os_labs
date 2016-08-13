#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64

#define PRINT 0

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
				printf("cd accepts only one argument\n");
			}
			else if ( chdir(tokens[1]) != 0){
				printf("Couldn't change directory to %s\n", tokens[1]);
			}
			continue;
		}

		if (strcmp(tokens[0], "exit") == 0){
			printf("Shell exiting\n");
			break;
		}

		int child = fork();
		if (child != 0){
			if (waitpid(child, NULL, 0) != child){
				printf("Error killing child\n");
			}
		}
		else{
			execvp(tokens[0], tokens);
		}

		// Freeing the allocated memory	
		for(i=0;tokens[i]!=NULL;i++){
			free(tokens[i]);
		}
		free(tokens);
	}
	 

}

				
