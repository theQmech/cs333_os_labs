#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>  
#include <fcntl.h>
#include <pthread.h>


# define NUM_THREADS 16  //number of threads
# define RUN_TIME 10.0  // runtime for each thread in seconds
# define SLEEP_TIME 1

struct threaddata {

    int threadid;
    char * hostname;
    char * port;

};


void error(char *msg)
{
    perror(msg);
    exit(0);
}



void * clientproc(void * t) {

    struct threaddata *td = (struct threaddata *) t;

    //printf("Thread %d started.\n", td->threadid);

    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];
    /* create socket, get sockfd handle */
    
    portno = atoi(td->port);

    struct timeval t1, t2;
    double elapsedTime = 0.0;

    while(elapsedTime < RUN_TIME){

        gettimeofday(&t1, NULL);
            //printf("hostname %s  port %d .\n", td->hostname, portno);
        sockfd = socket(AF_INET, SOCK_STREAM, 0);
         if (sockfd < 0)
            error("ERROR opening socket");

    /* fill in server address in sockaddr_in datastructure */

        server = gethostbyname( td->hostname);
         if (server == NULL) {
             fprintf(stderr, "ERROR, no such host\n");
             error("ERROR, no such host\n");
             exit(0);
         }
         bzero((char *) &serv_addr, sizeof(serv_addr));

         serv_addr.sin_family = AF_INET;
          bcopy((char *)server->h_addr, (char *)&serv_addr.sin_addr.s_addr, server->h_length);
         serv_addr.sin_port = htons(portno);

        // everything ahead has to be done by each thread
        /* connect to server */
        if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
            error("ERROR connecting");
        // specify file number
        int filenumber = 0;  // decide yourself
        char *dest = (char *)malloc(sizeof(char) * 4);
        sprintf(dest, "%d", filenumber);
        char filename[] = "get files/file";
        strcat(filename, dest);
        strcat(filename, ".txt\0");
        printf("%d - %s\n", td->threadid, filename);
        /* send user message to server */

        strcpy(buffer, filename); // so that none of further code changes

        // make a fire request to server
        n = write(sockfd, buffer, strlen(buffer) );
        if (n < 0)
            error("ERROR writing to socket");
        bzero(buffer, 256);

        // start reading from server
        n = read(sockfd, buffer, 255 );

        while (1) { // read from file in chunks of 255;
            if (n == 0) {
                break;
            }
            else if (n == -1) {
                error("Error reading from socket");
            }
            else {
                bzero(buffer, 256);
                n = read(sockfd, buffer, 255 );
            }

 

         }
        close(sockfd);

        sleep(SLEEP_TIME);
        gettimeofday(&t2, NULL);
        elapsedTime += (t2.tv_sec - t1.tv_sec);   
        //printf("Elapsed Time %f\n", elapsedTime);

    }
    printf("Thread %d  quitting\n", td->threadid);



    pthread_exit(NULL);

}


int main(int argc, char *argv[])
{

    int sockfd, portno, n;

    struct sockaddr_in serv_addr;
    struct hostent *server;

    char buffer[256];

    int rc, i; // what is this for

    if (argc < 3) {
        fprintf(stderr, "usage %s hostname port\n", argv[0]);
        exit(0);
    }

    pthread_t threads[NUM_THREADS];
    struct threaddata td[NUM_THREADS];

    printf("Starting all cleints \n");
    for (i = 0; i < NUM_THREADS ; i++) {
        td[i].threadid = i; 
        td[i].hostname = (char *) malloc(sizeof(char) * strlen(argv[1]));
        td[i].port = (char *) malloc(sizeof(char) * strlen(argv[2]));
        strcpy(td[i].hostname, argv[1]);
        strcpy(td[i].port, argv[2]);

        rc = pthread_create(&threads[i], NULL,
                            clientproc, (void *)&td[i]);

        if (rc) {
            error("Unable to create threads.");
            exit(-1);
        }
        printf("Thread %d started\n", i );

    }

    i = 0;
    for(i = 0; i < NUM_THREADS; i++){
        pthread_join(threads[i], NULL);
    }

    return 0;
}
