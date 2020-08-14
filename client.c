// Jeremiah Spears | 10/3/2019 | Intro To Networking Lab 1
// This file contains the client side of the proxy server
// This program functions by taking a given port and web address
// It will then connect to the proxy server with the website given
// and the server will forward an http request from the given site
// The client should be run on the cse02.cse.unt.edu server
// to run the program use ./a.out <port> <website address>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
 
 // function prototypes
int inet_pton(int af, const char *src, void *dst);
ssize_t read(int fd, void *buf, size_t count);
ssize_t write(int fd, const void *buf, size_t count);

int main(int argc, char *argv[])
{
    int sockfd, n;	// socket integer and 
    char recvline[4096000];	// string to hold http message from server
    struct sockaddr_in servaddr;	// sockaddr
	char webpage[500];	// string to hold webpage adr
	int port = 0;	// int to hold port
	
	
	if (argc == 3)
	{
		strcpy(webpage, argv[2]);	// setup webpage adr
		port = atoi(argv[1]);	// setup port
		printf("Webpage to get: %s\n", webpage);	// print webpage adr back to client
	}
	else
	{
		printf("Usage: %s <port> <website address>\n", argv[0]);	// print usage to user
		exit(1);	// exit if incorrect usage
	}

	// create socket to server
    /* AF_INET - IPv4 IP , Type of socket, protocol*/
    sockfd = socket(AF_INET, SOCK_STREAM, 0);

    bzero(&servaddr, sizeof(servaddr));	// reset servaddr
 
    servaddr.sin_family = AF_INET;	// set AF_INET
    servaddr.sin_port = htons(port); // Server port number 22000
 
    /* Convert IPv4 and IPv6 addresses from text to binary form */
	// 129.120.151.94 (cse01.cse.unt.edu)
	inet_pton(AF_INET, "129.120.151.94", &(servaddr.sin_addr));
 
    /* Connect to the server */
    if (connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1)
	{	
		perror("connection error\n");
		exit(1);	// exit on error
	}
	
	printf("Connected to server\n");	// print connected message
	
	write(sockfd, webpage, strlen(webpage)); // write to the client
	
	// wait for message from server
    while((n = read(sockfd, recvline, sizeof(recvline)) > 0))
    {
        printf("Message: %s", recvline); // print the received text from server
    }
	printf("\n");	// newline

	return 0;
}
