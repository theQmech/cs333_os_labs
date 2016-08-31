#include <stdlib.h>
#include <string>
#include <cstring>
#include <stdio.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <pthread.h>

using namespace std;

#define READ_SIZE 1024
#define MSG_SIZE 255
#define FILE_NO 0
#define MAX_FILE_ID 10000
#define MAX(a, b) ((a>b)? a:b) 
#define PRINT 0
#define PRINT_FILE 0
#define DEBUG 0

int NUM_THREADS;
int RUN_TIME;
int SLEEP_TIME;
int isRandom = 0;

struct threaddata
{
	int threadid;
	struct sockaddr_in *serv_addr;
	int requestCount;
	int runtime;
	int sleep_time;
	double avg_RT;
};

void error(string msg){
	perror(msg.c_str());
	exit(0);
	return;
}

void * clientproc(void * t) {

	struct threaddata *td = (struct threaddata *) t;

	int n, sockfd;
	
	char buffer[MAX(MSG_SIZE, READ_SIZE) +1];

	struct timeval t1, t2, req_start, req_end;
	double elapsedTime = 0.0;
	double curr_RT;
	td->requestCount = 0;
	td->avg_RT = 0.0;

	while (elapsedTime < td->runtime){

		gettimeofday(&t1, NULL);

		sockfd = socket(AF_INET, SOCK_STREAM, 0);
		if (sockfd == -1)
			error("ERROR opening socket");

		// everything ahead has to be done by each thread
		/* connect to server */
		if (connect(sockfd, (struct sockaddr*)td->serv_addr, sizeof(*td->serv_addr)) < 0){
			if (DEBUG) printf("T[%d]\tCan't connect to server\n", td->threadid);
			gettimeofday(&t2, NULL);
			elapsedTime += (t2.tv_sec - t1.tv_sec) + 
					(t2.tv_usec - t1.tv_usec)*0.000001;
			close(sockfd);
			continue;
		}
		if (DEBUG) printf("T[%d]\tconnect[%d]\n", td->threadid, sockfd);
		//	error("ERROR connecting");
		// specify file number

		int filenumber = 0;  // decide yourself
		if (isRandom)
			filenumber = rand() % MAX_FILE_ID;
		
		/* send user message to server */
		bzero(buffer, MSG_SIZE+1);
		sprintf(buffer, "get files/foo%d.txt", filenumber);
		if (PRINT) printf("Thread %d: `%s`\n", td->threadid, buffer);

		// make a file request to server
		gettimeofday(&req_start, NULL);
		n = write(sockfd, buffer, strlen(buffer) );
		if (n <= 0){
			error("Unable to make request to server\n");
			gettimeofday(&t2, NULL);
			elapsedTime += (t2.tv_sec - t1.tv_sec) + 
				(t2.tv_usec - t1.tv_usec)*0.000001;
			close(sockfd);
			continue;
			//error("ERROR writing to socket");
		}
		if (DEBUG) printf("T[%d]\t`%s`\n", td->threadid, buffer);

		// start reading from server
		bzero(buffer, READ_SIZE+1);
		while ((n = read(sockfd, buffer, READ_SIZE)) > 0){
			if (PRINT_FILE) printf("%s", buffer);
			bzero(buffer, READ_SIZE+1);
		}
		if (n == 0) if (PRINT) printf("File recieved\n");
		else if (n == -1){
			error("Error downloading file\n");
			gettimeofday(&t2, NULL);
			elapsedTime += (t2.tv_sec - t1.tv_sec) + 
				(t2.tv_usec - t1.tv_usec)*0.000001;
			close(sockfd);
			continue;
			//error("Error reading from socket");
		}
		if(DEBUG) printf("T[%d]\tRecieved\n", td->threadid);

		close(sockfd);
		
		gettimeofday(&req_end, NULL);
		curr_RT = req_end.tv_sec - req_start.tv_sec + 
					(req_end.tv_usec - req_start.tv_usec)*0.000001;
		td->avg_RT = td->avg_RT + curr_RT;
		td->requestCount = td->requestCount + 1 ;

		sleep(td->sleep_time);
		gettimeofday(&t2, NULL);
		elapsedTime += (t2.tv_sec - t1.tv_sec) + 
				(t2.tv_usec - t1.tv_usec)*0.000001;
	}

	printf("Thread %d exit. Count is %d.\n", td->threadid, td->requestCount);
	pthread_exit(NULL);
}


int main(int argc, char *argv[]){

	int sockfd, portno, n;

	struct sockaddr_in serv_addr;
	struct hostent *server_t;

	char buffer[MSG_SIZE+1];

	int rc, i; // what is this for

    if (argc < 7) {
        fprintf(stderr, "usage: %s hostname port nthread time sleep mode\n", argv[0]);
        exit(0);
    }

	NUM_THREADS = atoi(argv[3]);
	RUN_TIME = atoi(argv[4]);
	SLEEP_TIME = atoi(argv[5]);
	portno = atoi(argv[2]);

	if ( strcmp(argv[6], "random") == 0) {
		isRandom = 1;
	}
	// threads resources
	pthread_t *threads = new pthread_t[NUM_THREADS] ;
	struct threaddata *td = new threaddata[NUM_THREADS] ;

	/* fill in server address in sockaddr_in datastructure */
	server_t = gethostbyname(argv[1]);
	if (server_t == NULL ) {
		error("ERROR, no such host");
		exit(0);
	}
	if (server_t->h_addr==NULL){
		fprintf(stderr, "T[%d]\tserver_t->h_addr is NULL\n", td->threadid);
		exit(0);
	}
	bzero((char *) &serv_addr, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	bcopy((char *)server_t->h_addr, (char*)&serv_addr.sin_addr.s_addr, server_t->h_length);
	serv_addr.sin_port = htons(portno);

	printf("Starting all clients \n");
	for (i = 0; i < NUM_THREADS ; i++){
		td[i].threadid = i;
		td[i].serv_addr = &serv_addr;
		td[i].runtime = RUN_TIME;
		td[i].sleep_time = SLEEP_TIME;
		td[i].requestCount = 0;
		void *arg = (void*)&td[i];
		rc = pthread_create(&threads[i], NULL, clientproc, arg);
		if (rc){
			error("Unable to create threads.");
			exit(1);
		}
		printf("Thread %d started\n", i );
	}

	for (i = 0; i < NUM_THREADS; i++) {
		pthread_join(threads[i], NULL);
	}

	int sum = 0;
	double AVT = 0.0;

	for ( i = 0; i < NUM_THREADS ; i++) {
		printf("Thread %d: %d, %f sec\n", i, td[i].requestCount, td[i].avg_RT);
		sum += td[i].requestCount;
		AVT += td[i].avg_RT;
	}
	AVT = AVT/sum;
	
	printf("\nDone ! \n");
	double throughput = sum * 1.0 / (RUN_TIME);
	printf("Throughput is %f reqs/sec.\n", throughput);
	printf("Average response time = %f\n", AVT);

	delete[] threads;
	delete[] td;
	return 0;
}

