/*
	Jeremiah Spears | 10/3/2019 | Intro To Networking Lab 1
	This file contains the server side of the proxy server
	In the server the server waits for a client to connect and then receives the web address from the client
	Once the address is given it parses it if needed and then gets the ip using gethostbyname
	Then it connects to the website and sends a get http request 
	It then receives the response and forwards that response back to the client

	If the server has already received the site recently it will have stored the site
	in it's cache and can forward the cache file to the user instead of requesting the site.
	Only 5 sites can be stored in the cache at a time.
	The server also has a blacklist for banned sites within a specificed time interval.
*/

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

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
		bzero(str, 4096000);	// reset str
		bzero(webpage, 500);		// reset webpage

		printf("Waiting for client..\n");	// waiting for client message
		FILE *listFile; // list file
		FILE *blackListFile; // blacklist file

		listFile = fopen("list.txt", "a+"); // open file
		blackListFile = fopen("blacklist.txt", "r");

		// make sure files are open
		if (listFile == NULL)
		{
			perror("Could not open listFile\n");
			exit(1);
		}
		if (blackListFile == NULL)
		{
			perror("Could not open blacklist\n");
			exit(1);
		}

		/* Accepts an incoming connection */
		conn_fd = accept(listen_fd, (struct sockaddr*)NULL, NULL);
		printf("Client connected\n");	// client connected message
	  
		read(conn_fd, webpage, sizeof(webpage));	// read message from client
		printf("%s\n", webpage); // print the received text from serve
		
		char msg [500];	// string to hold message from website
		
		char line[600];	// str to hold line
		int cached = 0;	// cached false

		// if found in blacklist with correct time
		// 	 message client
		//   continue
		int blackList = 0;	// blacklist false

		// read blacklist file
		while (fgets(line, sizeof line, blackListFile) != NULL)
		{
			// check if webpage is in blacklist
			if (strstr(line, webpage) != NULL)
			{
				printf("%s found in blacklist... checking the time\n", webpage);

				long long timeB = 0;	// time before
				long long timeA = 0;	// time after 
				long long currentDT = 0; // current time 

				// get current date/time
				char currentDTStr[75];	 // current date/time str
				time_t t = time(NULL);	// get date/time
				struct tm tm = *localtime(&t);	// format date/time as localtime
				
				// date format YYYYMMDDhhmmss
				// format date/time
				sprintf(currentDTStr, "%04d%02d%02d%02d%02d%02d", 
					tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

				int i = 0;	// counter variable
				char *temp = strtok(line, " ");	// split at " "
				char *strArr[2];	// creat arr for strtok

				// read the rest of the line
				while (temp != NULL)
				{
					strArr[i++] = temp; // put date/times in arr 0 and 1
					temp = strtok(NULL, " ");
				}
				
				// convert to long longs
				timeB = atoll(strArr[1]);
				timeA = atoll(strArr[2]);	
				currentDT = atoll(currentDTStr);

				// check if current DT is in DT range to block site
				if (currentDT > timeB && currentDT < timeA)
				{
					blackList = 1;	// set to true
					break; // exit loop
				}
			}
		}
		fclose(blackListFile);	// close blacklist

		// if found in blacklist
		if (blackList)
		{
			sprintf(str, "%s website is blocked\n", webpage);
		}


		// see if the website is already saved in cache
		if (!blackList)
		while (fgets(line, sizeof line, listFile) != NULL)
		{
			if (strstr(line, webpage) != NULL)
			{
				printf("%s found in cache\n", webpage);
				cached = 1;	// if site is found set to true
				break;	// exit loop if site is found in cache
			}
		}

		// if the site is already saved
		if (cached && !blackList)
		{
			// get site from cache
			bzero(str, 4096000);	// reset str
			//sprintf(str, "Test!\n");

			char cachedFileName[75];
			char *temp = strchr(line, ' ');
			temp++; // skip the ' ' char
			strtok(temp, "\n"); // remove trailing newline char
			sprintf(cachedFileName, "%s.txt", temp);	// format fileName
			
			FILE *cacheFile;	// file
			cacheFile = fopen(cachedFileName, "r");	// open cacheFile for given website

			// make sure file is open
			if (cacheFile == NULL)
			{
				perror("Could not open cacheFile\n");
				exit(1);
			}	

			fseek(cacheFile, 0, SEEK_END);	// go to end of file
			long size = ftell(cacheFile);	// get size of file
			fseek(cacheFile, 0, SEEK_SET);	// go back to beginning of file

			fread(str, 1, size, cacheFile);	// read entire file into str
			str[size] = 0;	// null terminate str

			fclose(cacheFile);	// close file
		}
		else if (!blackList)
		{
			// else get site and add it to cache if given code 200

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
			
			fclose(listFile);
			
			// check if site sent html code 200
			// and if it did add the site to the cache
			if (strstr(str, "1.1 200") != NULL)
			{
				printf("Html code 200 received!\n");
				
				FILE *listFile; // list file
				listFile = fopen("list.txt", "a+"); // open file
				if (listFile == NULL)
				{
					perror("Could not open listfile\n");
					exit(1);
				}	

				fseek(listFile, 0, SEEK_SET);	// go back to beginning of file

				int count = 0; // count of lines in file
				while (fgets(line, sizeof line, listFile) != NULL)
				{
					count++; // increment count
				}
				fseek(listFile, 0, SEEK_SET);	// go back to beginning of file

				// check if file has 5 saved sites already
				if (count >= 5)
				{
					printf("More than 5 lines\n");
					FILE *t;
					t = fopen("temp.txt", "a+"); // temp file to copy list
					// make sure file is open
					if (t == NULL)
					{
						perror("Could not open tempFile\n");
						exit(1);
					}	
					
					// need to delete cache file also check if files are open

					int i = 0; // counter
					// read through listFile
					while (fgets(line, sizeof line, listFile) != NULL)
					{
						// check if first line
						if (i != 0)
						{
							fputs(line, t);
						}
						else
						{
							// read first line and delete file with name
							char cachedFile[75];	// file name for cached site file

							strtok(line, "\n");	// remove training newline char
							char *temp = strchr(line, ' ');
							temp++; // skip the ' ' char
							sprintf(cachedFile, "%s.txt", temp);	// format fileName
							remove(cachedFile);	// remove cacheFile
						}
						i++;
					}
					remove("list.txt");	// delete list.txt
					rename("temp.txt", "list.txt");	// rename temp file to list.txt
					fclose(t);	// close t
					fclose(listFile);	// close listFile
				}

				FILE *listFile2; // list file
				listFile2 = fopen("list.txt", "a+"); // open file
				if (listFile2 == NULL)
				{
					perror("Could not open listfile\n");
					exit(1);
				}	

				char listEntry[600];	// new list entry to add
				char newCachedFile[75];	// new file name for cached site file
				time_t t = time(NULL);	// get date/time
				struct tm tm = *localtime(&t);	// format date/time as localtime
				
				// date format YYYYMMDDhhmmss
				// format list entry
				sprintf(listEntry, "%s %04d%02d%02d%02d%02d%02d", 
					webpage, tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
				sprintf(newCachedFile, "%04d%02d%02d%02d%02d%02d.txt", 
					tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);

				fprintf(listFile2, "%s\n", listEntry); // add list entry to file
				fclose(listFile2);	// close file

				FILE *cacheFile;	// new cacheFile
				
				cacheFile = fopen(newCachedFile, "a");	// open/create new cacheFile

				// make sure file is open
				if (cacheFile == NULL)
				{
					perror("Could not open cacheFile\n");
					exit(1);
				}	

				fprintf(cacheFile, "%s", str);	// save webpage in new cacheFile
				fclose(cacheFile);	// close file
			}

			// else don't cache site
			
			close(web_fd);	// close connection with website
		}
		
		write(conn_fd, str, strlen(str)); // write to the client
		close (conn_fd); //close the connection with client
		
		printf("Client disconnected\n\n");	// disconnection message
	}
	
	return 0;
}
