#!/usr/bin/python 
#CS372 - Intro to Networking
# Project 2
#
# Usage: python2 ftpclient.py [hostName] [connectPort] [command] [dataPort]  if command == -l
# Usage: python2 ftpclient.py [hostName] [connectPort] [command] [fileName] [dataPort]  if command == -g
#
# This was my first time using python so excuse the C type behavior 
#


import sys 
import socket
from struct import *
import re
import os



#########################################################
#Function: argumentCheck
#Description: Checks the arguments passed into the program
#	      to make sure they are usable. Exits if not
#Parameters: connectPort, command, dataPort
#Returns:  Returns -1 on error 
#######################################################
def argumentCheck(connectPort, command, dataPort):
 


	if not connectPort.isdigit():
	    print "connect port error"
	    return -1
	
	if not dataPort.isdigit():
	    print "data port error"
	    return -1
	
	connectPort = int(connectPort)
	dataPort = int(dataPort)
	
	
	if connectPort > 65536 or connectPort < 0:
		print("Error. Please keep port number between 0 and 65535")
		return -1

	if dataPort > 65536 or dataPort < 0:
		print("Error. Please keep port number between 0 and 65535")
		return -1
		

		
	if '-l' not in command and '-g' not in command:
			print "Incorrect Command. Usage: -l or -g"	
			return -1
		
		
		

	return 0

#########################################################
#Function: socketSetup
#Description: This function just sets up all the usable sockets
#	      needed for the program and error checks them
#Parameters:   serverName, connectPort, command, fileName, dataPort
#Returns:  Exits on failure or moves onto connectionSetup
#######################################################
def socketSetup(serverName, connectPort, command, fileName, dataPort):
	
	
	#set up connectSocket 
	try:
		connectSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0) #IPv4, TCP, 0 protocol 
	except:
		print "Error setting up TCP Connect Socket"
		sys.exit(1)

	#set up dataSocket
	try:
		acceptSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
	except:
		print "Error setting up TCP Accept Socket"
		sys.exit(1)
		
	try:
		dataSocket = socket.socket(socket.AF_INET, socket.SOCK_STREAM, 0)
	except:
		print "Error setting up TCP Data Socket"
		sys.exit(1)
		
		
	connectionSetup(connectSocket, acceptSocket, dataSocket, connectPort, serverName, command, fileName,dataPort)

	
#########################################################
#Function: conectionSetup
#Description: Sets-up the connectionSocket and gets hostName
#	    Puts together the complete command to be sent to server	
#	   and retrieves receiving message. If -l is specified and 
#	  server okays it...program moves to list action. If -g is 
#	    specified, fileAction.
#Parameters: conenctSocket, acceptSocket, dataSocket, connectPort, ServerName, command, fileName, dataPort
#Returns: No return. Continues into listAction and FileAction
#######################################################	
def connectionSetup(connectSocket, acceptSocket, dataSocket, connectPort, serverName, command, fileName,dataPort):

	
	try:
		connectSocket.connect((serverName,connectPort))
	except:
		print "Error connecting to Server"
		sys.exit(1)
	else:
		print "Connected to Client"
		
	
	hostName = socket.gethostname();
	
	
	if fileName is None:
		fullCommand = hostName + " " + command + " " + dataPort 
	else:
		fullCommand = hostName + " " + command + " " + fileName + " " + dataPort 
	
	print fullCommand
	
	
	try:
		connectSocket.send(fullCommand)
	except:
		print "Error with send"
		sys.exit(1)
	else:
		print "FullCommand sent"
	
	
	bufferData = connectSocket.recv(1024)
	

	print bufferData 	

	if "Error" in bufferData:
		sys.exit(1)
	else:
		if "-l" in command:
			listAction(connectSocket,dataSocket, acceptSocket, dataPort, serverName)
		if "-g" in command:
			fileAction(connectSocket, dataSocket, acceptSocket, dataPort, fileName)
	
		
		
		
	
#########################################################
#Function: listAction
#Description: This function sets up dataSocket to bind and listen
#	   ,makes acceptSocket be the accepted socket which will receive
#	  the file list from the server. 
#Parameters: connectSocket, dataSocket, acceptSocket, dataPort, serverName
#Returns: Exits on finish
#######################################################

def listAction(connectSocket, dataSocket, acceptSocket, dataPort, serverName):

	dataSocket.bind(('', int(dataPort))) #binds dataSocket to dataPort specified 
	dataSocket.listen(1) #there is no reason to listen to more than 1 
		

	
	
	#acceptSocket,addr is the return pair of dataSocket.accept()
	acceptSocket, addr = dataSocket.accept()

	#Message sent to console
	print "Sending directory contents to ", serverName, " ", dataPort

	#Main loop that goes while file names are being transferred
	while 1:


		#4096 bytes of data retrieved	
		listing = acceptSocket.recv(4096)
		
	
		print listing
		
		#if data stops then we can exit out and close the sockets
		if not listing:
			acceptSocket.close()
			connectSocket.close()
			sys.exit(0)
		



	

#########################################################
#Function: fileAction
#Description: Starts up the dataSocket for the file transfer
#	     ,checks for file errors. Receives the file size
#		then uses this to run receive file loop
#Parameters: connectSocket, dataSocket, acceptSocket, dataPort,fileName
#Returns: Exits 0 in end 
#######################################################

def fileAction(connectSocket, dataSocket, acceptSocket, dataPort, fileName):


	dataSocket.bind(('', int(dataPort)))
	dataSocket.listen(1)
	


	#the main loop that runs the receiving process
	while True:  
		acceptSocket,addr = dataSocket.accept() #socket.accept() requires a socket, then address 
		
		fileCheck = connectSocket.recv(1024)   #error messages are sent to connectPort
	
		#Server is set up to send this in the error message so we can use to track it	
		if "FILE NOT FOUND" in fileCheck:   
			print fileCheck
			sys.exit(1)
		
		else:
			print fileCheck
			name = fileName


		#Next the file size is received. Filecheck holds this
		fileCheck = connectSocket.recv(1024)

		print "File Size"
		print fileCheck

	
		#msg will hold the received data 
		msg = ''	

		#fileCheck also has added on \0s which we do not want since we cant convert that to int	
		length = int(fileCheck.strip('\0'))



#		print repr(length)		

	
		#since we know the length of the file we can use this as the loop condition
		#when the length of msg which is gathering the data is better than length...its done
		while length > len(msg):
#			acceptSocket, addr = dataSocket.accept()
			data  = acceptSocket.recv(length - len(msg))
			if data == '':   # in the event of a closed socket. Exit 
				sys.exit(1)   
			msg = msg + data
		#	print msg

		print "Transfer finished"
	
		#This uses the os module to check whether the file exists in the directory		
		if os.path.isfile(name):

			#if name is already in existance, you get to rename it :D 
			print "File already exists. Enter a new name for this file"
			answer = raw_input();

			#opens the file
			f = open(answer, 'w')
			
		#else just open the file which the use specified if not there
		else:
			f = open(name, 'w')
			
		
			
#		print msg		
		f.write(msg)
		break	

	f.close()
	

	print "File transfer complete"

	#Close the sockets we dont need 
	acceptSocket.close()
	connectSocket.close()
	sys.exit(0)
	

#########################################################
#Function: main
#Description: Checks the arguments passed in and then 
#             passes appropriate arguments to setup 
#Parameters: -
#Returns:  Exits with 0 
#######################################################

def main():

#	print(sys.argv)

	#check number of arguments. Exit if less than 4 or more than 6
	if len(sys.argv) < 4 or len(sys.argv) > 6:
		print "Incorrect number of parameters: Usage: ftclient [hostname] [port] [command] [port]"
		print "or"
		print "ftpclient [hostname] [port] [command] [filename] [port]"
		sys.exit(1)
	
	#Set up variables from passed in arguments
	if len(sys.argv) == 5:
		serverName = sys.argv[1]
		connectPort = sys.argv[2]
		command = sys.argv[3]	
		dataPort = sys.argv[4]
		fileName = None     #fileName is none because its not being passed in as an argument
	elif len(sys.argv) == 6:
		serverName = sys.argv[1]
		connectPort = sys.argv[2]
		command = sys.argv[3]
		fileName = sys.argv[4]
		dataPort = sys.argv[5]
	

	#Error check the ports 
	
	
	if argumentCheck(connectPort, command, dataPort) == -1:
		print "Parameters incorrect. Make sure ports are int and command is either -l or -g"
		sys.exit(1)
	else:
		connectPort = int(connectPort)
		#dataPort = int(connectPort)
	
	
	

	
	
	#Moving onto setting the sockets up and then carrying on with the program 
	socketSetup(serverName, connectPort, command, fileName, dataPort)
	

	
	
	
	sys.exit(0)


	

			
	

	
#Declare main. Checks whether executed as the top level file  	
if __name__ == "__main__":
    main()	
