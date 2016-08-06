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

int NUM_THREADS;
int RUN_TIME;
int SLEEP_TIME;
int isRandom = 0;

struct threaddata
{
	int threadid;
	char * hostname;
	char * port;
	int * requestCount;
	double avg_RT;
};

void error(char *msg)
{
	perror(msg);
	exit(0);
}

void * clientproc(void * t) {

	struct threaddata *td = (struct threaddata *) t;

	int sockfd, portno, n;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[MAX(MSG_SIZE, READ_SIZE)+1];

	portno = atoi(td->port);

	struct timeval t1, t2, req_start, req_end;
	double elapsedTime = 0.0;
	double curr_RT;
	*td->requestCount = 0;
	td->avg_RT = 0.0;

	while (elapsedTime < RUN_TIME) {

		gettimeofday(&t1, NULL);

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd < 0)
			error("ERROR opening socket");

		/* fill in server address in sockaddr_in datastructure */

		server = gethostbyname(td->hostname);
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
		if (isRandom)
			filenumber = rand() % MAX_FILE_ID;
		
		char *request = (char *)malloc(sizeof(char)* (MSG_SIZE+1));
		sprintf(request, "get files/file%d.txt", filenumber);
		printf("Thread %d: %s\n", td->threadid, request);
		/* send user message to server */

		strcpy(buffer, request); // so that none of further code changes

		// make a file request to server
		gettimeofday(&req_start, NULL);
		n = write(sockfd, buffer, strlen(buffer) );
		if (n < 0)
			error("ERROR writing to socket");
		bzero(buffer, READ_SIZE+1);

		// start reading from server
		while ((n = read(sockfd, buffer, READ_SIZE)) > 0){
			bzero(buffer, 256);
			n = read(sockfd, buffer, 255 );
		}
		if (n == 0) printf("File recieved\n");
		else if (n == -1) error("Error reading from socket");
		
		close(sockfd);
		
		gettimeofday(&req_end, NULL);
		curr_RT = req_end.tv_sec - req_start.tv_sec + 
					(req_end.tv_usec - req_start.tv_usec)*0.001;
		td->avg_RT = td->avg_RT + curr_RT;
		*td->requestCount = *td->requestCount + 1 ;

		sleep(SLEEP_TIME);
		gettimeofday(&t2, NULL);
		elapsedTime += (t2.tv_sec - t1.tv_sec) + (t2.tv_usec - t1.tv_usec)*0.001;
	}

	printf("Thread %d exit. Count is %d.\n", td->threadid, *td->requestCount);
	pthread_exit(NULL);
}


int main(int argc, char *argv[]){

	int sockfd, portno, n;

	struct sockaddr_in serv_addr;
	struct hostent *server;

	char buffer[MSG_SIZE+1];

	int rc, i; // what is this for

    if (argc < 7) {
        fprintf(stderr, "usage: %s hostname port nthread time sleep mode\n", argv[0]);
        exit(0);
    }

	NUM_THREADS = atoi(argv[3]);
	RUN_TIME = atoi(argv[4]);
	SLEEP_TIME = atoi(argv[5]);

	if ( strcmp(argv[6], "random") == 0) {
		isRandom = 1;
	}
	// threads resources
	pthread_t threads[NUM_THREADS] ;
	struct threaddata td[NUM_THREADS] ;
	int numRequests[NUM_THREADS] ;

	printf("Starting all clients \n");
	for (i = 0; i < NUM_THREADS ; i++){
		td[i].threadid = i;
		td[i].hostname = (char *) malloc(sizeof(char) * strlen(argv[1]));
		td[i].port = (char *) malloc(sizeof(char) * strlen(argv[2]));
		strcpy(td[i].hostname, argv[1]);
		strcpy(td[i].port, argv[2]);
		td[i].requestCount = &numRequests[i];
		rc = pthread_create(&threads[i], NULL,
							clientproc, (void *)&td[i]);

		if (rc) {
			error("Unable to create threads.");
			exit(-1);
		}
		printf("Thread %d started\n", i );

	}

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	int sum = 0;
	double AVT = 0.0;

	for ( i = 0; i < NUM_THREADS ; i++) {
		printf("Thread %d: %d, %f sec\n", i, numRequests[i], td[i].avg_RT);
		sum += numRequests[i];
		AVT += td[i].avg_RT;
	}
	AVT = AVT/sum;
	
	printf("\nDone ! \n");
	double throughput = sum * 1.0 / (RUN_TIME);
	printf("Throughput is %f reqs/sec.\n", throughput);
	printf("Average response time = %f\n", AVT);

	return 0;
}
