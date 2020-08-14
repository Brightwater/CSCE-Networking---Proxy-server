/*
 Jeremiah Spears | 10/3/2019 | Intro To Networking Lab 1
 This file contains the server side of the proxy server
In the server the server waits for a client to connect and then receives the web address from the client
Once the address is given it parses it if needed and then gets the ip using gethostbyname
Then it connects to the website and sends a get http request 
It then receives the response and forwards that response back to the client
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// function prototype
ssize_t write(int fd, const void *buf, size_t count);

// function to get given webaddress's ip and
// connect to webaddress
// and return the web_fd
int webConnect(char *address)
{
	struct sockaddr_in addr;	// sockaddr
	struct hostent *hp;			// hostent
	int fd;	// web_fd int
	
	// gethostbyname from given address
	if ((hp = gethostbyname(address)) == NULL){
		perror("gethostbyname");
		exit(1);	// exit on error
	}
	
	bcopy(hp->h_addr, &addr.sin_addr, hp->h_length);	// copy h_addr
	
	addr.sin_port = htons(80);	// set port
	addr.sin_family = AF_INET;	// set AF_INET
	
	fd = socket(PF_INET, SOCK_STREAM, 0);	// create socket to website
	
	if(connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_in)) == -1){
		perror("connect");
		exit(1);	// exit on error

	}
	
	return fd;	// return web_fd
}
 
int main(int argc, char *argv[])
{
	char str[4096000];	// str for holding message to client
	int port = 0;	// port number declaration
	int listen_fd, conn_fd, web_fd;	// fd ints
	struct sockaddr_in servaddr;	// sockaddr for server
	
	// check if arguments given is 2
	if (argc == 2)
	{
		port = atoi(argv[1]);	// setup port
	}
	else
	{
		printf("Usage: %s <port>\n", argv[0]);	// print usage to user
		exit(1);	// exit program
	}

	/* AF_INET - IPv4 IP , Type of socket, protocol*/
	listen_fd = socket(AF_INET, SOCK_STREAM, 0); // create socket

	bzero(&servaddr, sizeof(servaddr));	// reset servaddr
	
	servaddr.sin_family = AF_INET;	// set AF_INET
	servaddr.sin_addr.s_addr = htons(INADDR_ANY);	// setup addr
	servaddr.sin_port = htons(port); 	// set port
	

	/* Binds the above details to the socket */
	if ((bind(listen_fd,  (struct sockaddr *) &servaddr, sizeof(servaddr)) == -1))
	{
		perror("bind error\n");
		exit(1);
	}

	/* Start listening to incoming connections */
	if ((listen(listen_fd, 10) == -1))
	{
		perror("listen error\n");
		exit(1);
	}

	char webpage[500];	// string to hold web address
	
	while(1)
	{
		printf("Waiting for client..\n");	// waiting for client message
		/* Accepts an incoming connection */
		conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
		printf("Client connected\n");	// client connected message
	  
		read(conn_fd, webpage, sizeof(webpage));	// read message from client
		printf("%s\n", webpage); // print the received text from serve
		
		char msg [500];	// string to hold message from website

		// check if website has / after domain 
		if (strchr(webpage, '/') != NULL)
		{
			// if it does get the part after the char before /
			// and store it in directory
			char *directory = strchr(webpage, '/');
			// skip the / char
			directory++;

			// parse the part of the address before the / char
			strtok(webpage, "/");

			// setup get request message with a directory
			sprintf(msg, "GET /%s HTTP/1.1\r\nHost: %s\r\n\r\n", directory, webpage);
		}
		else
		{
			// setup get request message without a directory
			sprintf(msg, "GET / HTTP/1.1\r\nHost: %s\r\n\r\n", webpage);
		}
		
		web_fd = webConnect(webpage);	// get web_fd from webConnect function
		
		write(web_fd, msg, strlen(msg));	// send get request to website
		
		bzero(str, 4096000);	// reset str
		
		read (web_fd, str, 4095999);	// read message from website
		
		close(web_fd);	// close connection with website
		
		write(conn_fd, str, strlen(str)); // write to the client
		close (conn_fd); //close the connection with client
		
		printf("Client disconnected\n");	// disconnection message
	}
	
	return 0;
}
