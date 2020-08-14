Jeremiah Spears | 10/3/2019 | Intro To Networking Lab 1

client.c
To compile: gcc -Wall client.c
Run parameters: ./a.out <port> <website>
In the client the client connects to the server and sends the web address to It
Then it awaits the reply from the server and prints the reply (http message)

server.c
To compile: gcc -Wall server.c
Run parameters: ./a.out <port>
In the server the server waits for a client to connect and then receives the web address from the client
Once the address is given it parses it if needed and then gets the ip using gethostbyname
Then it connects to the website and sends a get http request 
It then receives the response and forwards that response back to the client
