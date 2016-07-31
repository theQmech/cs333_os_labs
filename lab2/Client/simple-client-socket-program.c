#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h> 
#include <fcntl.h>
void error(char *msg)
{
    perror(msg);
    exit(0);
}


int main(int argc, char *argv[])
{
    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    /* create socket, get sockfd handle */

    portno = atoi(argv[2]);
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd < 0)
        error("ERROR opening socket");

    /* fill in server address in sockaddr_in datastructure */

    server = gethostbyname(argv[1]);
    if (server == NULL) {
        fprintf(stderr, "ERROR, no such host\n");
        exit(0);
    }
    bzero((char *) &serv_addr, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    bcopy((char *)server->h_addr,
          (char *)&serv_addr.sin_addr.s_addr,
          server->h_length);
    serv_addr.sin_port = htons(portno);

    /* connect to server */

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
        error("ERROR connecting");

    /* ask user for input */

    printf("Please enter the command: ");
    bzero(buffer, 256);
    fgets(buffer, 255, stdin);

    // extract file name from server
    int msglen = (unsigned) strlen(buffer);
    char * filename = (char *)malloc(sizeof(char) * (msglen - 11));
    filename = memcpy(filename, &buffer[10], msglen - 11 );
    filename[msglen - 11] = '\0';
    printf("downloading file %s\n", filename);
    // char buffer[] = "get files/file0.txt";
    /* send user message to server */

    n = write(sockfd, buffer, strlen(buffer) );
    if (n < 0)
        error("ERROR writing to socket");
    bzero(buffer, 256);
    /* read file from server */
    FILE * ofile = fopen(filename, "w");
    
    //printf("76 \n");

    int outfd = fileno(ofile);
    if (outfd < 0)
        error("Error opening destination file.");

    //printf("80");
    n = read(sockfd, buffer, 255 );

    while (1) { // read from file in chunks of 255
        //printf("entered loop");

        //printf("n is %d", n);
        if (n==0){
            break;
        }
        else if(n==-1){
            error("Error reading from socket");  
        }
        else{
        //printf("91");
        if (write(outfd, buffer, n) < 0)
            error("Error writing to file");
        //printf("written");
        bzero(buffer, 256);
        n = read(sockfd, buffer, 255 );
        }
    }
    close(outfd);
    close(sockfd);
    return 0;
}
