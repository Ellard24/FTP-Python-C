/***************************************
 * Author: Ellard Gerritsen van der Hoop
 * Class: CS 372
 * Project 2
 *
 *
 * ftpserve.cpp
 * *************************************/




#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <dirent.h>    //will be used to look through directory
#include <vector>
#include <netdb.h>
#include <fstream>
#include <sys/stat.h>


#define BACKLOG 10


using namespace std;


//prototypes
void sockSetAndCheck(int &sockfd);
void portAndServerSetup(int &portNumber, char *argv[], struct sockaddr_in &serverAddress);
void bindAndListen(int &sockfd, struct sockaddr_in &serverAddress);
void listFiles(char **args, vector<string> &files, int accept_fd);

int checkCommand(char *recvBuffer, char **args);
void fileLocator(vector<string> &files);
//void fileTransfer(char *recvBuffer);
void fileTransfer(char **args, int accept_fd, vector<string> &files);
/********************************************************
 * Function: sockSetAndCheck
 * Description: This function sets the socket and then checks for 
 * 		errors. If error occurs, problem exits out
 * Parameters:  sockfd 
 * Returns: No return value. Void function
 * **************************************************/

void sockSetAndCheck(int &sockfd){
	
	//sets the socket and looks for error
	sockfd = socket(AF_INET, SOCK_STREAM, 0);   //AF_INET == IPv4, SOCK_STREAM = TCP
		
	if (sockfd < 0){
		cout << "Error. We have socket problems :(" << endl;
		exit(1);
	}

}

/************************************************************
 * Function: portAndServerSetup
 * Description: This function checks the specified port number taken from argument,
 * 		if good it will set the serverAddress information, otherwise exit.
 * Parameters: int portNumber, char *argv[], struct sockaddr_in
 * Returns: No return value. Void function.
 * ********************************************************/
void portAndServerSetup(char *argv[], struct sockaddr_in &serverAddress, int ftp){

	//This is needed to convert from string to int since it came from command line 
	
	int portNumber; 

	if (ftp == 0){
		portNumber = atoi(argv[1]);
	}
	else
	{
		portNumber = ftp; 
	}
	//This is hte range of acceptable ports. Though obviously reserved pors should be avoided
	if (portNumber < 0 || portNumber > 65536)
	{
		cout << "Error. Port needs to be between 0 and 65535" << endl;
		exit(1);


	}
	
	printf("PortNumber: %d\n", portNumber);
	
	serverAddress.sin_family = AF_INET; //IPv4
	serverAddress.sin_addr.s_addr = INADDR_ANY;  // binds to all interfaces
	serverAddress.sin_port = htons(portNumber); //converts portNumber to acceptable value used for sin_port

}




/************************************************************
 * Function: bindAndListen
 * Description: This functions binds the socket descriptor, serverAddress
 * 		and then checks for errors. Exits on error. 
 * 		Then it starts listening on the socket with the specified backlog 
 * Parameters: sockfd, struct sockaddr_in &serverAddress
 * Returns: No return value. Void function
 * ********************************************************/

void bindAndListen(int &sockfd, struct sockaddr_in &serverAddress){

	if (bind(sockfd, (struct sockaddr*)&serverAddress, sizeof(serverAddress)) < 0){
		cout << "Error. Binding problem" << endl;
		exit(1); 
	}

	//use of macro
	listen(sockfd, BACKLOG);  

}
//we need to check just the first part to see if command is legal 
/************************************************************
 * Function: checkCommand
 * Description: This functions takes the received command from the 
 * 		user and parses it into arguments to be used later
 * 		.Then it checks to see what command was passed
 * Parameters: char *recvBuffer, char **args
 * Returns: Returns 0, 1, 2 depending on command 
 * ********************************************************/

int checkCommand(char *recvBuffer, char **args)
{
	char str1[] = "-g"; //holds the get command 
	char str2[] = "-l"; //holds the list command
	char temp[100];

	//this isnt needed but strtok can do some really bad things to a string
	strcpy(temp, recvBuffer);
	
	int i; 
	int tempCount = 0;

	char *p;

	//Good old tokenizing. This splits the recvBuffer into arguments 
	for (p = strtok(recvBuffer, " "); p != NULL; p = strtok(NULL, " "))
	{
		//Removes newline
		int i = strlen(p) - 1;   
		if ((i > 0) && (p[i] == '\n'))
			p[i] = '\0';

		//adds char to args array and increments element number
		args[tempCount] = p;
		tempCount++; 



	}

	//compares arguments with known commands. Returns appropriate value	
	if (strcmp(args[1], "-g") == 0)
		return 1;
	if (strcmp(args[1], "-l") == 0)
		return 2;
	else
		return 0;

}


//This will put all the current files in directory into vector 
/************************************************************
 * Function: fileLocator
 * Description: This functions takes the files vector and adds
 * 		all the file names of files in the current 
 * 		working directory.  
 * Parameters: vector<string> &files
 * Returns: No return value. Void function
 * ********************************************************/

void fileLocator(vector<string> &files)
{


	DIR *dir;
	struct dirent *dp;

	dir = opendir(".");

	//this loops through the directory and pushes fileNames into vector 
	while ((dp = readdir(dir)) != NULL)
		files.push_back(dp->d_name);
	
	closedir(dir);
	
	int i = 0;
	
	//debug 
//	for (i = 0; i < files.size(); i++)
//		cout << files.at(i) << endl;
//
	

}

/************************************************************
 * Function: fileTransfer
 * Description: This functions transfers the appropriate file 
 * 		contents over to the user. A new socket is set up
 * 		and used for this. Error checks file name as well
 * 		and cleans up where needed
 * Parameters: char **args, int accept_fd, vector<string> &files
 * Returns: No return value. Void function
 * ********************************************************/

void fileTransfer(char **args, int accept_fd, vector<string> &files)
{
	int datafd;	

	//the required structs for setting up another connection 
	struct sockaddr_in client_addr;
	struct hostent *server;
	
	sockSetAndCheck(datafd);
	

	//Deal with the arguments 
	char *hostName = args[0];
	char *command = args[1];
	char *fileName = args[2];
	int portNumber = atoi(args[3]);

	
	//set up the client_addr with IPv4, the passed in portNumber for the datafd
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(portNumber); //need to fix this
	client_addr.sin_addr.s_addr = INADDR_ANY;

	server = gethostbyname(args[0]);
	
	if (connect(datafd, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0 ){
		printf("Error connecting to server\n");
		exit(1);
		

	}
	

	//We are going to loop through the entire files vector and compare the name of the 
	//sent fileName from the user to verify it actually exists 
	int fileCheck = 0;
	for (int i = 0; i < files.size();i++)
	{
//		printf("FileName: %s", fileName);
//		cout << "   Vector:  " << files.at(i)  << endl;
		if (strcmp(fileName, files.at(i).data()) == 0)
			fileCheck = 1;

	}
	
	//fileCheck of 0 is an error. Sents error message over accept_fd not the datafd
	if (fileCheck == 0)
	{
		char msg[200]; 		//will hold error message
		bzero(msg, sizeof(msg));  //zero it out
		printf("File not found. Sending error message to %s : %d\n", hostName, portNumber);  //prints to screen 
		sprintf(msg, "%s: %d says FILE NOT FOUND", hostName, portNumber);	//appends info to msg to be sent out
	
		send(accept_fd, msg, sizeof(msg),0);
	}
	else{		//same as before but this time with a success message 
		char msg[200];
		bzero(msg, sizeof(msg));
		printf("Sending %s to %s : %d\n", fileName, hostName, portNumber);
		sprintf(msg, "Receiving %s from %s: %d", fileName, hostName, portNumber);
		send(accept_fd, msg, sizeof(msg),0); 
	}

	//open file name specified earlier "read only". Error check 
	FILE *fp = fopen(fileName, "r");

	if (fp == NULL)
		printf("Error: File not found");
	else
		printf("File opened\n");	
	


	//stackoverflow.com/questions/8236/how-do-you-determine-the-size-of-a-file-in-c
	//Using sys/stat.h . Create struct that you pass filName into and it returns st.st_size which is the file size
	struct stat st;
	int fileSize;
	if (stat(fileName, &st) != 0)
		printf("Error");
	else{
		fileSize = st.st_size;
//		printf("St:  %d\n", st.st_size);

	}
	printf("File Size: %d\n",fileSize);

	//Need a carrier of our file size message. Zero the memory, attach fileSize to it
	char fileS[10];
	bzero(fileS, sizeof(fileS));
	sprintf(fileS, "%d", fileSize);


	///SEnding file size
	send(accept_fd, fileS, sizeof(fileS), 0);
	
	//Variables for our send loop. Sending a char at a time
	int ch;
	char toSEND[1];

	
	//While loop goes until end of file, sending a char each time
	while ((ch=getc(fp)) != EOF){
	//	printf("Inside transfer\n");
		toSEND[0] = ch;
		send(datafd, toSEND, 1,0);
	}		 
	fclose(fp);
	close(datafd);

	printf("Finished Transfer\n");
	printf("Waiting for new connections\n");
	return;

}


/************************************************************
 * Function: listFiles
 * Description: This functions is similiar to fileTransfer but
 * 		instead sends the name of all the files in the 
 * 		current servers working directory using the vector.
 * 		Uses datafd to accomplish this and closes when done
 * Parameters: char **args, vector<string> &files, int accept_fd
 * Returns: No return value. Void function
 * ********************************************************/

void listFiles(char **args, vector<string> &files, int accept_fd)
{
	//This will be the FTP data socket
	int datafd;	

	//Another sockaddr_in /hostent structs for 2 TCP connection
	struct sockaddr_in client_addr;
	struct hostent *server;
	
	sockSetAndCheck(datafd);
	

	//Deal with the arguments 
	// host name = 0
	// command = 1
	// dataPort = 2
	char *hostName = args[0];
	char *command = args[1];
	int portNumber = atoi(args[2]);

	
	client_addr.sin_family = AF_INET;
	client_addr.sin_port = htons(portNumber);
	client_addr.sin_addr.s_addr = INADDR_ANY;

	//Need to specify server from which user is 
	server = gethostbyname(args[0]);
	

	//Connect to client with datafd socket
	if (connect(datafd, (struct sockaddr *) &client_addr, sizeof(client_addr)) < 0 ){
		printf("Error connecting to server\n");
		exit(1);
		

	}
	
	//Sending the newline makes it a bit easier to see the list of files for the client
	char newline[] = "\n";	

	//Loop through the file vector and send their names over to client along with newlines between them
	for (int i = 0; i < files.size(); i++)
	{
	//	cout << files[i] << endl;
		send(datafd, files[i].data(), files[i].size(), 0);
		send(datafd, newline, sizeof(newline),0);
	}

	printf("Waiting for new connections\n");
	
	
	close(datafd);

}

/************************************************************
 * Function: main 
 * Description: Creates the variables needed for message simulation
 * 		, zeroes the memory of serverAddress, calls the appropriate
 * 		sequence of socket,bind,listen, accept, send/recv,
 * 		goes to appropriate command action, and closes socket on exit
 * Parameters: argc, argv[]
 * Returns: Returns 0 on exit
 * ********************************************************/

int main(int argc, char *argv[]) {
	//Files Descriptors for listening and Accept
	int sockfd, accept_fd;
	


	char *args[5];

	
	socklen_t clilen; //Size of client address
	
	//This vector will hold the names of all the files in the server's directory
	vector<string> files; 

	//This function will actually fill up the vector of the file names 
	fileLocator(files);


	//All the necessary structs. 1 for server and 1 for client. Will hold all important connection data 
	struct sockaddr_in serverAddress, clientAddress; 

	//Bzero works just as well as memset. User preference 
	bzero((char *)&serverAddress, sizeof(serverAddress));
	
	//Remember the order: socket, bind, listen, accept, send/recv


	//Argument check. 
	if(argc < 2) { 
		cout << "Error. Arguments incorrect. Usage: ftpserve [port]" << endl;
		exit(1); 
	}
	else{
		portAndServerSetup(argv, serverAddress, 0);
	}

	sockSetAndCheck(sockfd);

	bindAndListen(sockfd, serverAddress);


	cout << "Server Open" << endl;
	clilen = sizeof(clientAddress);

	//Keep the server running
	for(;;) { //Just set some random value that will keep the server constantly running until it gets a sigint
		int option;
		int FTPport;
		char command[100];
		char file[100]; 
		

		
		accept_fd = accept(sockfd, (struct sockaddr *) &clientAddress, &clilen);

		
		cout << "Connected to Client" << endl;

		char recvBuffer[1024];
		char sendBuffer[100];

		//zeroing out the sendBuffer
		memset(sendBuffer, '\0', sizeof(sendBuffer));


		//zeroing out the recvBuffer
		bzero(recvBuffer, sizeof(recvBuffer));

		//Option will track the error status. 
		option = recv(accept_fd,recvBuffer, sizeof(recvBuffer),0);
	
	
		if (option < 0){
			printf("Err with recv\n");
			exit(1);
		}
		else
			printf("received message: %s\n", recvBuffer);
		
	
	
		//this will check the command sent from user and parse the arguments
		option = checkCommand(recvBuffer, args);
	
//		printf("option after checkCommand: %d\n", option);

		//Return value of checkCommand decides course of action here: 1 is Get file, 2 is list, 0 is Error
		if (option == 1){ 
			strcpy(sendBuffer, "Get Requested");
//			printf("Get command\n");
			option = send(accept_fd, sendBuffer, sizeof(sendBuffer),0);
			fileTransfer(args, accept_fd, files);

		}
		else if (option == 2){  //list
			strcpy(sendBuffer, "List requested");
//			printf("SendBuffer:      %s\n", sendBuffer);
			option = send(accept_fd, sendBuffer, sizeof(sendBuffer),0);
			strcpy(command, sendBuffer);
			listFiles(args, files ,accept_fd);


		}
		else if (option == 0 ){
			strcpy(sendBuffer, "Error incorrect Command");
			option = send(accept_fd, sendBuffer, sizeof(sendBuffer),0);
			
//			printf("Option: %d\n", option);
		}
		
		else {
			close(accept_fd);
		} 
	}


    close(sockfd);


	return 0;
}
