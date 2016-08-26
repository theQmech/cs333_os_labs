#include <string>
#include <iostream>
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
#include <memory.h>
#include <sys/stat.h> 
#include <fcntl.h>
#include <pthread.h>
#include <queue>

using namespace std;

#define MAX_CONN 5
#define READ_SIZE 1024
#define MSG_SIZE 255
#define FILE_NO 0
#define MAX_FILE_ID 10000
#define MAX(a, b) ((a>b)? a:b) 
#define PRINT 0

queue<int> req_q;
pthread_mutex_t q_lock = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t empty = PTHREAD_COND_INITIALIZER;
pthread_cond_t full = PTHREAD_COND_INITIALIZER;
int QSIZE, NTHREAD;

void sendFile(int sock_fd, char * fileaddress); 

void error(string msg){
    perror(msg.c_str());
    exit(1);
}

void enqueue(int sockfd){
	// to be called only with the lock held
	if (/*lock not held*/false){
		printf("Lock not held in enqueue\n");
		exit(1);
	}
	bool was_empty = req_q.empty();
	req_q.push(sockfd);
	if (was_empty) pthread_cond_signal(&empty);
	return;
}

int dequeue(){
	// to be called only with the lock held
	if (/*lock not held*/false){
		printf("Lock not held in dequeue\n");
		exit(1);
	}
	int sz = req_q.size();
	int ret_val = req_q.front();
	req_q.pop();
	if (QSIZE != 0 && sz == QSIZE) pthread_cond_signal(&full);
	return ret_val;
}

void *cli_handl(void *t){
	/* client handler thread */
	int t_id = *(int *)t, n, newsockfd;

    char buffer[MSG_SIZE+1];
	bzero(buffer, MSG_SIZE+1);

	while(true){

		pthread_mutex_lock(&q_lock);
		while (req_q.empty()){
			pthread_cond_wait(&empty, &q_lock);
		}
		newsockfd = dequeue();
		pthread_mutex_unlock(&q_lock);

		n = read( newsockfd, buffer, MSG_SIZE );
		
		if (n < 0) {
		    perror("ERROR reading from socket");
		    exit(1);
		}
	
		int msglen = (unsigned) strlen(buffer);
		char * fileaddress = (char *)malloc(sizeof(char) * (msglen - 4));
		fileaddress = (char *)memcpy(fileaddress, &buffer[4], msglen - 4 );
		
		sendFile(newsockfd, fileaddress);
		close(newsockfd);
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

		bzero(buffer, READ_SIZE+1);

    }
	if (n == 0) {if (PRINT) printf("File %s sent successfully\n", fileaddress);}
	else if (n < 0) {printf("Error reading from file socket");}
    close(input_fd);
}

int main(int argc, char *argv[]){

    int sockfd, newsockfd, portno;
   	socklen_t clilen;
    char buffer[MSG_SIZE+1];
    struct sockaddr_in serv_addr, cli_addr;
    int n; // base size of clients;

    pid_t *clientsPID = (pid_t *)malloc(sizeof(pid_t) * 4); //
    int *clientStatus = (int *)malloc(sizeof(int) * 4);

    if (argc != 4) {
        fprintf(stderr, "usage: %s <portno> <n_thread> <queue_size>\n", argv[0]);
        exit(1);
    }
	NTHREAD = atoi(argv[2]);
	QSIZE = atoi(argv[3]);

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

	pthread_t *rthread_id = new pthread_t[NTHREAD];
	int *t_rc = new int[NTHREAD];
	for (int i=0; i<NTHREAD; ++i){
		t_rc[i] = i+1;
		pthread_create(&rthread_id[i], NULL, cli_handl, (void*)&t_rc[i]);
	}

	while (1) {
		// accept connections and then lock&append
		// 
		// if we first lock, then accept and append
		// then accept will block leading and threads 
		// will block as well(due to queue lock not being released)

		newsockfd = accept(sockfd, (struct sockaddr *) &cli_addr, &clilen);
        if (newsockfd <= 0)
			error("ERROR on accept");

		pthread_mutex_lock(&q_lock);
		while(req_q.size() == QSIZE){
			pthread_cond_wait(&full, &q_lock);
		}
		enqueue(newsockfd);
		pthread_mutex_unlock(&q_lock);

    }

	for (int i=0; i<NTHREAD; ++i){
		pthread_join(rthread_id[i], NULL);
	}
    printf("Server Quitting \n" );

}

