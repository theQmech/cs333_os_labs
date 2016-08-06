#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/sendfile.h>
#include <netinet/in.h>
#include <string.h>
#include <memory.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <fcntl.h>

void error(char *msg)
{
    perror(msg);
    exit(1);
}


int reap(int numClients, pid_t * clientsPID, int * clientStatus) {
    int i, alldead = 1;
    for (i = 0; i < numClients ; i++) {
        if ( clientStatus[i] ) { // children not dead or status not available
            alldead = 0;
            // call wait
            clientsPID[i] = waitpid(clientsPID[i], &clientStatus[i] , WCONTINUED | WNOHANG | WUNTRACED);
            if (clientStatus[i] == 0)
                printf(" Reaped child process %d \n", clientsPID[i]);
        }
    }

    if (alldead == 1 && numClients > 0) {
        // do something here
        printf("All child processes reaped.\n");
        alldead = 0;
    }
    else{

        alldead = 1;
    }

    return alldead;
}



void sendFile(int sock_fd, char * fileaddress) {

    printf("Started loading file %s\n", fileaddress );
    int input_fd = open(fileaddress, O_RDONLY);
    if (input_fd < 0)
        error("Error loading file. Invalid fileaddress");
    char buffer[512];
    int n = read(input_fd, buffer, 511 ) ;
    if (n < 0)
        error("Error reading file");
    while (1 ) { // read from file in chunks of 255

        if (n == 0) {
            break;
        }
        else if (n < 0) {
            error("Error reading from socket");
        }
        else {

            if (write(sock_fd, buffer, n ) < 0)
                error("Error writing to socket");

            bzero(buffer, 512);
            n = read(input_fd, buffer, 511 ) ;
        }

    }
    close(input_fd);
    // open the file cheack for errors
    // sendfile check for errror
}

int main(int argc, char *argv[])
{
    int sockfd, newsockfd, portno, clilen;
    char buffer[256];
    struct sockaddr_in serv_addr, cli_addr;
    int n, numClients = 0, arraySize = 4; // base size of clients;

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

    listen(sockfd, 5);
    clilen = sizeof(cli_addr);
    printf("Server started\n");
    /* accept a new request, create a newsockfd */
    pid_t pid;
    int status = 1;
    while (status) {

        status = reap(numClients, clientsPID, clientStatus);

        newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd < 0)
            error("ERROR on accept");

        numClients++;
        pid = fork();

        if (pid != 0) { // parent process
            //call waitpid to reap dead children
            if (numClients > arraySize ) {
                arraySize = arraySize * 2;
                clientsPID = realloc(clientsPID, arraySize * sizeof(int));
                clientStatus = realloc(clientStatus, arraySize * sizeof(int));
            }

            clientsPID[numClients - 1 ] = pid;
            clientStatus[numClients - 1] = 1;

            close(newsockfd);

        }
        else {
            /* read message from client */
            /* This is the client process */
            bzero(buffer, 256);
            n = read( newsockfd, buffer, 255 );

            if (n < 0) {
                perror("ERROR reading from socket");
                exit(1);
            }
            int msglen = (unsigned) strlen(buffer);
            char * fileaddress = (char *)malloc(sizeof(char) * (msglen - 4));
            fileaddress = memcpy(fileaddress, &buffer[4], msglen - 4 );
            // fileaddress[msglen - 5] = '\0';
            close(sockfd);
            sendFile(newsockfd, fileaddress);
            exit(0);
        }


        // reap(numClients, clientsPID, clientStatus);

    }

    printf("Server Quitting \n" );

}

