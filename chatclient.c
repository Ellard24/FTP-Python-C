/***************************************************
 * Author: Ellard Gerritsen van der Hoop
 * Class: CS 372
 * Project 1 
 *
 * chatclient.c 
 * ***********************************************/




#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>

#define MAXHANDLE 11
#define MAXMESSAGELEN 500

/********************************************************
 *Function: sockSetAndCheck
 *Description: Sets the socket and checks for errors. Exits
		on error
 *Parameters:sockfd
 *Returns: No return value. Void function
**********************************************************/


void sockSetAndCheck(int *sockfd){
	*sockfd = socket(AF_INET, SOCK_STREAM, 0);  //IPv4, TCP, 0 for flags

	if (sockfd < 0){
		printf("We have a socket error :(\n");
		exit(1);
	}
}

/*************************************************************************************************
 *Function: portAndServerSetup
 *Description: Takes the port number from argument when starting program,
		converts it to a usable int, then checks to see if it falls within
		the appropriate range. Exits on error. Then it sets up the serverAddress
		with the onverted portNumber, and AF_INET. server is then set to specified 
		host name from argument and checks to see that its not null.
 *Parameters: int portNumber, char *argv[], struct sockaddr_in * serverAddress, struct hostent **server
 *Returns: 
*************************************************************************************************/


void portAndServerSetup(int *portNumber, char *argv[], struct sockaddr_in *serverAddress, struct hostent **server){

	*portNumber = atoi(argv[2]);


	if (*portNumber < 0 || *portNumber > 65536)
	{
		printf("Error. Port needs to be between 0 and 65535\n");
		exit(1);


	}

	serverAddress->sin_family = AF_INET; //IPv4
	serverAddress->sin_port = htons(*portNumber);
	
	*server = gethostbyname(argv[1]); //Grabs the server information and holds it into the struct

	if (*server == NULL)
	{
		printf("Error. Can not find host\n");
		exit(1);
	}
}

/********************************************************
 *Function: getName
 *Description: Gets the name from the user so that it can be sent to
		server along with specified message later on
 *Parameters: char **name
 *Returns: No return value
**********************************************************/


void getName(char **name){
	
	//allocate memory for name
	*name = malloc(11);

	//set all to 0 before getting stdin
	bzero(*name, sizeof(*name));
	printf("What name do you want to use?\n");
	fgets(*name, 10, stdin);

	printf("Name: %s\n", *name);
	char *pos;

	//Replace newline with null term
	if((pos=strchr(*name, '\n')) != NULL) { 
		*pos = '\0';
	}
	
	
}

/********************************************************
 *Function: messageSystem
 *Description: This function will take care of the intereaction between client and server.
		It creates the variables needed to send and retrieve messages, it zeroes the bytes,
		checks for quit message, prints error when failure occurs. 
 *Parameters: int *sockfd, char *name
 *Returns: No return value. Void function
**********************************************************/


void messageSystem(int *sockfd, char *name){
	int n = 0;
	char sendBuffer[MAXMESSAGELEN];
	char appendedMessage[MAXMESSAGELEN + 10]; //The typed message and username together
	char recvBuffer[MAXMESSAGELEN];
	
	//Take in input from fgets and append name to input message
	printf("Type your message: ");
	bzero(sendBuffer,500);
	fgets(sendBuffer,500,stdin); 
	sprintf(appendedMessage,"%s> %s", name, sendBuffer); // sprintf combines name and sendBuffer so we get username and message in one string
    
	if(strstr(sendBuffer, "\\quit") == 0) {  //checks for quit message
      
      		n = send(*sockfd,appendedMessage,strlen(appendedMessage),0);
     
      		bzero(recvBuffer,500); //zeroes the bytes 
      		n = recv(*sockfd,recvBuffer,500,0); 
      		
		//N == 0 happens when server disconnects 
		if(n == 0) {
        		printf("Disconnected From Server\n");
        		return;
      		}
		// less then 0 is an error
		else if (n < 0)
		{
			printf("Error with receiving message\n");
			exit(1);
		}
		//this statement checks for if server types \quit. If not, it prints message
      		else {
			if (strstr(recvBuffer, "\\quit") == 0)
				printf("Server> %s",recvBuffer);
			else{
        			printf("Server disconnected\n");
				exit(1);
			}
      		}
    	}
    	else { 
      		exit(1);	
    	}





}


/********************************************************
 *Function:main
 *Description: Sets up the variables needed to run simulation. It 
		then calls the appropriate functions in correct sequence
		socket, bind, connect, then starts message system on 
		successful connection
 *Parameters: int argc, char *argv[]
 *Returns: Returns 0 on program end
**********************************************************/


int main(int argc, char *argv[]) {
	
	int sockfd; //Listen on this one (file descriptor)
	int portNumber; //Port number
	int n; //Contains number of characters to read/write
	struct sockaddr_in serverAddress;
	struct hostent *server; //Defines the host computer's information
	
	char *name;


	getName(&name);  //sets up name for handler

	sockSetAndCheck(&sockfd); //creates socket and checks for error

	//printf("Made it pass socket\n"); DEBUG MESSAGE

	//We need to 0 out the serverAddress before we can set up the struct
	bzero((char *) &serverAddress, sizeof(serverAddress));
	portAndServerSetup(&portNumber, argv, &serverAddress, &server);
	
	//Bcopy copies the bytes from source(server) to destination(serverAddress) 
	bcopy((char *)server->h_addr, (char *)&serverAddress.sin_addr.s_addr, server->h_length);

//	printf("made it past Port/Server stuff\n"); DEBUG MESSAGE
  

	//Connect returns value of -1 when error
	if (connect(sockfd,(struct sockaddr *) &serverAddress,sizeof(serverAddress)) < 0){
		printf("Error connecting to server\n");
		exit(1);
	}
	else
		printf("Connection Successful\n");

  	//Starts the message loop 
	while(1) { //loop is endless so we can keep this going until error occurs or client/server disconnect
		messageSystem(&sockfd, name);
		
  	}

  	//Close listening socket
  	close(sockfd);
  	return 0;
}
