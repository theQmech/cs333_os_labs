#include <sys/types.h> 
#include <sys/wait.h> 
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <string.h>
#include <memory.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <pthread.h>

#define MAX_CONN 5
#define READ_SIZE 1024
#define MSG_SIZE 255
#define FILE_NO 0
#define MAX_FILE_ID 10000
#define MAX(a, b) ((a>b)? a:b) 
#define PRINT 0

void error(char *msg)
{
    perror(msg);
    exit(1);
}

void * reap(void *t){

	//reap all zombie process
	int zomb;
	while(1){
		while((zomb = waitpid(-1, NULL, WNOHANG)) >0){
			if (PRINT)
			printf("[%d] child reaped\n", zomb);
		}
	}
	pthread_exit(NULL);
}

void sendFile(int sock_fd, char * fileaddress) {

	if (PRINT)
    printf("Started loading file %s\n", fileaddress );
    int input_fd = open(fileaddress, O_RDONLY);
    if (input_fd < 0)
        error("Error loading file. Invalid fileaddress");
    char buffer[READ_SIZE+1];
    int n;

	while ((n = read(input_fd, buffer, READ_SIZE )) > 0){
		//read in chunks of READ_SIZE

		if (write(sock_fd, buffer, n ) < 0)
			error("Error writing to socket");
		
		//crucial part
		sleep(1);

		bzero(buffer, READ_SIZE+1);

    }
	if (n == 0) {if (PRINT) printf("File %s sent successfully\n", fileaddress);}
	else if (n < 0) {printf("Error reading from file socket");}
    close(input_fd);
}

int main(int argc, char *argv[]){

    int sockfd, newsockfd, portno, clilen;
    char buffer[MSG_SIZE+1];
    struct sockaddr_in serv_addr, cli_addr;
    int n; // base size of clients;

    pid_t *clientsPID = (pid_t *)malloc(sizeof(pid_t) * 4); //
    int *clientStatus = (int *)malloc(sizeof(int) * 4);

    if (argc < 2) {
        fprintf(stderr, "ERROR, no port provided\n");
        exit(1);
    }

    /* create socket */

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* fill in port number to listen on. IP address can be anything (INADDR_ANY) */

    bzero((char *) &serv_addr, sizeof(serv_addr));
    portno = atoi(argv[1]);
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = INADDR_ANY;
    serv_addr.sin_port = htons(portno);

    /* bind socket to this port number on this machine */

    if (bind(sockfd, (struct sockaddr *) &serv_addr,
             sizeof(serv_addr)) < 0)
        error("ERROR on binding");

    /* listen for incoming connection requests */

    listen(sockfd, MAX_CONN);
    clilen = sizeof(cli_addr);
    printf("Server started on port %d\n", portno);
    /* accept a new request, create a newsockfd */
    pid_t pid, zomb;

	pthread_t rthread_id;
	int t_rc = pthread_create(&rthread_id, NULL, reap, NULL);
    
	while (1) {

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd <= 0)
			error("ERROR on accept");

        pid = fork();

        if (pid != 0) { // parent process
            //call waitpid to reap dead children
			if (PRINT)
            printf("[%d] child forked\n", pid);
			close(newsockfd);
        }
        else {
            /* read message from client */
            /* This is the client process */
            close(sockfd);
            bzero(buffer, MSG_SIZE+1);
            n = read( newsockfd, buffer, MSG_SIZE );

            if (n < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            int msglen = (unsigned) strlen(buffer);
            char * fileaddress = (char *)malloc(sizeof(char) * (msglen ));
            fileaddress = memcpy(fileaddress, &buffer, msglen );

            sendFile(newsockfd, fileaddress);
			close(newsockfd);
            exit(0);
        }


    }

	pthread_join(t_rc, NULL);
    printf("Server Quitting \n" );

}

