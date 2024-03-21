// HEADER FILE: myftp.h

/* An FTP CLIENT Program Written in C */
// NAME: Linnea P. Castro
// DATE: 10 NOV 2023
// COURSE: CS 360
// ASSIGNMENT: Final Project

//LIBRARIES TO INCLUDE
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <ctype.h>
#include <errno.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <dirent.h>

//DEFINE STATEMENTS
#define MY_PORT_NUMBER 18181
#define MY_PORT_NUMBER_CLIENT "18181"
#define BUFFER_SIZE 1024

#define DEBUG 1

//SERVER FUNCTION PROTOTYPES
void serverFunction(int listenfd);
int dataConnectionServerFunction(int connectfd);
int rlsServerFunction(int connectfd, int dcConnectfd);
int rcdServerFunction(int connectfd, char *buffer);
int getServerFunction(int dcConnectfd, char *buffer, int connectfd);
int putServerFunction(int connectfd, int dcConnectfd, char *buffer);

//CLIENT FUNCTION PROTOTYPES
void clientFunction(char *address);
int lsFunction(void);
int cdFunction(char *secondToken);
int dataConnectionClientFunction(int socketfd, char *buffer, char *address);
int rlsClientFunction(int socketfd, int dcsocketfd, char *buffer);
int rcdClientFunction(int socketfd, char *secondToken, char *buffer);
int getClientFunction(char *secondToken, char *buffer, char *address, int socketfd);
int showClientFunction(char *secondToken, char *buffer, char *address, int socketfd);
int putClientFunction(int socketfd, char *buffer, char *address, char *secondToken);
int quitClientFunction(int socketfd, char *buffer);
