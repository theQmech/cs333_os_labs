#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>

#define READ_SIZE 1024
#define MSG_SIZE 255
#define FILE_NO 0
#define MAX_FILE_ID 10000
#define MAX(a, b) ((a>b)? a:b) 
#define PRINT 1

void error(char *msg)
{
	perror(msg);
	exit(0);
}

int main(int argc, char *argv[]){

	int sockfd, portno, n;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[READ_SIZE+1];

	int rc, i; // what is this for

    if (argc < 5) {
        fprintf(stderr, "usage: %s <file> <server_address> <port> <display/nodisplay>\n", argv[0]);
        exit(0);
    }

	portno = atoi(argv[3]);
	printf("%d\n", portno);
	char DISP[] = "display\0";
	int disp = strcmp(argv[4], DISP);

	if( (sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		error("ERROR opening socket");

	/* fill in server address in sockaddr_in datastructure */

	if( (server = gethostbyname(argv[2])) == NULL){
		fprintf(stderr, "ERROR, no such host\n");
		error("ERROR, no such host\n");
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));

	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
	serv_addr.sin_port = htons(portno);

	if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
		error("ERROR connecting");
		// specify file number

	if (PRINT)
		printf("%s\n", argv[1]);
	/* send user message to server */

	strcpy(buffer, argv[1]); // so that none of further code changes

	// make a file request to server
	n = write(sockfd, buffer, strlen(buffer) );
	if (n < 0)
		error("ERROR writing to socket");
		bzero(buffer, READ_SIZE+1);

	// start reading from server
	while ((n = read(sockfd, buffer, READ_SIZE)) > 0){
		if (disp==0) printf("%s", buffer);
		bzero(buffer, 256);
	}
	if (n == 0) if (PRINT) printf("File recieved\n");
	else if (n == -1) error("Error reading from socket");
		
	close(sockfd);

	return 0;
}
