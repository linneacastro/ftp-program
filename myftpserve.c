// SERVER FILE: myftpserve.c

/* An FTP SERVER Program Written in C */
// NAME: Linnea P. Castro
// DATE: 10 NOV 2023
// COURSE: CS 360
// ASSIGNMENT: Final Project

// LIBRARIES TO INCLUDE
#include "myftp.h"

/*
void serverFunction(int listenfd);
int dataConnectionServerFunction(int connectfd);
int rlsServerFunction(int connectfd, int dcConnectfd);
int rcdServerFunction(int connectfd, char *buffer);
int getServerFunction(int dcConnectfd, char *buffer, int connectfd);
int putServerFunction(int connectfd, int dcConnectfd, char *buffer);
*/

// MAIN FUNCTION BODY
int main(int argc, char const *argv[]){

    // VARIABLES
    int listenfd = 0;
    //int serverCheck = 0;
    int setSockOptCheck = 0;

    // CREATE A SOCKET AND ERROR CHECK
    listenfd = socket(AF_INET, SOCK_STREAM, 0); // Pg. 1155 in textbook.  The socket system call "installs a telephone".
    if (listenfd == -1){ // Returns -1 on error. 
        fprintf (stderr, "Error: %s\n", strerror(errno)); 
        return (errno);
    } 

    // INITIALIZE SOCKET AND ERROR CHECK
    setSockOptCheck = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // This will allow you to immediately reuse this port, very important to do this
    if (setSockOptCheck == -1){
        fprintf (stderr, "Error: %s\n", strerror(errno)); 
        return (errno); 
    }

    // CHECKING COMMAND LINE ARGS... IF SERVER
    if (argc == 1){ // If user only specified program name, no file given
        serverFunction(listenfd);
    }

    // TOO MANY COMMAND LINE ARGS
    if (argc >= 2){ // 2 or more command line args given
        printf("Error: Too many command line arguments.  Please try running the program again.\n");
        return 0;
    }

    return 0;
}

//SERVER FUNCTION BODY
void serverFunction(int listenfd){
    int connectfd;
    unsigned int length;

    char hostName[NI_MAXHOST];

    int bindCheck = 0;
    int listenCheck = 0;
    int forkStatus = 0;
    int hostEntry = 0;
    int writeCheck = 0;
    int bytesRead = 0;
    char buffer[BUFFER_SIZE] = {0};

    char fileBuffer[BUFFER_SIZE] = {0};

    struct sockaddr_in servAddr;
    struct sockaddr_in clientAddr;

    memset(&servAddr, 0, sizeof(servAddr));
    servAddr.sin_family = AF_INET;
    servAddr.sin_port = htons(MY_PORT_NUMBER);
    servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    // BIND AND ERROR CHECK
    bindCheck = bind(listenfd, (struct sockaddr *) &servAddr, sizeof(servAddr));
    if (bindCheck < 0){
        fprintf (stderr, "Error: %s\n", strerror(errno)); 
        exit(1); 
    }

    // LISTEN AND ERROR CHECK
    listenCheck = listen(listenfd, 4); // Takes 4 attempts before blocking. 
                                       // Listen sets up a connection queue one level deep
    if (listenCheck == -1){
        fprintf (stderr, "Error: %s\n", strerror(errno)); 
    }

    // GET LENGTH FOR ACCEPT FUNCTION (NEXT STEP)
    length = sizeof(struct sockaddr_in); // Get length to pass into accept function

    // INFINITE LOOP FOR ACCEPT
    while(1){ 
        // CONNECT AND ERROR CHECK
        connectfd = accept(listenfd, (struct sockaddr *) &clientAddr, &length); // On success, returns a file descriptor
        // accept waits (blocks) until a connection is established by a client
        if (connectfd == -1){
            fprintf (stderr, "Error: %s\n", strerror(errno)); 
            exit(1);
        }

        forkStatus = fork();

        if (forkStatus == -1){
            fprintf (stderr, "Error: %s\n", strerror(errno)); 
            exit(1);
        }

        if (forkStatus != 0){ // This is the PARENT
            close(connectfd); // Close parent 

            // GET HOSTNAME FROM CLIENT AND ERROR CHECK
            hostEntry = getnameinfo((struct sockaddr*)&clientAddr, sizeof(clientAddr), hostName, sizeof(hostName), NULL, 0, NI_NUMERICSERV);
            if (hostEntry != 0){
                fprintf (stderr, "Error: %s\n", gai_strerror(hostEntry)); 
                exit(1);
            }

            //CONFIRM CONNECTION AND PRINT HOSTNAME DETAILS
            printf("Child %d: Connection accepted from host %s\n", forkStatus, hostName);
            fflush(stdout); // Fflush here is essential!
            continue; 
        }

        if (forkStatus == 0){ // This is the CHILD
            int dcConnectfd = -1; // this must be init outside the while loop, otherwise it resets btwn rcvng D and L commands from client

            while(1){
                int bufferLength = 0;
                char commandLetter;
                //int dclistenfd = 0;
                int readCheck = 0;

                int i = 0;
                while ((readCheck = read(connectfd, &buffer[i], 1) > 0)){
                    if (buffer[i] == '\n'){
                        break;
                    }
                    i++;
                }

                //printf("Value of readcheck: %d\n", readCheck);
                if (readCheck == -1){
                    fprintf (stderr, "E%s\n", strerror(errno));
                    exit(1); 
                }

                if (DEBUG) printf("Received <%s> from client.\n", buffer);
                buffer[i] = '\0'; // get rid of newline
                if (DEBUG) printf("New buffer contents (removed newline): <%s>\n", buffer);

                // EXTRACT FIRST LETTER OF BUFFER AS THE COMMAND
                commandLetter = buffer[0];

                // RCVD 'D' FROM CLIENT
                if (commandLetter == 'D'){
                    dcConnectfd = dataConnectionServerFunction(connectfd);
                    if (dcConnectfd == -1){
                        int lengthOfError = 0;
                        lengthOfError = strlen(strerror(errno));
                        printf("Eerror with data connection <%s>\n", strerror(errno)); 
                        write(connectfd, "E", 1);
                        write(connectfd, strerror(errno), lengthOfError);
                        write(connectfd, "\n", 1);
                    }
                }

                // RCVD 'L' FROM CLIENT
                if (commandLetter == 'L'){
                    int rlsCheck = 0;
                    rlsCheck = rlsServerFunction(connectfd, dcConnectfd);
                    if (rlsCheck == -1){
                        printf("Eerror calling the rlsServerFunction. No data connection established.\n");
                        char *errorMessage = "Eerror calling the rlsServerFunction. No data connection established.";
                        int lengthOfError = 0;
                        lengthOfError = strlen(errorMessage);
                        write(connectfd, errorMessage, lengthOfError);
                        write(connectfd, "\n", 1);
                    }
                }

                // RCVD 'C' FROM CLIENT
                if (commandLetter == 'C'){
                    printf("I got your C.  Now going to call rcd function.\n");
                    int rcdCheck = 0;
                    rcdCheck = rcdServerFunction(connectfd, buffer);
                    if (rcdCheck == -1){
                        int lengthOfError = 0;
                        lengthOfError = strlen(strerror(errno));
                        printf("Eerror with rcd <%s>\n", strerror(errno)); 
                        write(connectfd, "E", 1);
                        write(connectfd, strerror(errno), lengthOfError);
                        write(connectfd, "\n", 1);
                    }
                }

                // RCVD 'G' FROM CLIENT
                if (commandLetter == 'G'){
                    int getCheck = 0;
                    getCheck = getServerFunction(dcConnectfd, buffer, connectfd);
                    if (getCheck == -1){
                        printf("Eerror performing server get function.\n");
                        char *errorMessage = "Eerror calling the getServerFunction.";
                        int lengthOfError = 0;
                        lengthOfError = strlen(errorMessage);
                        write(connectfd, errorMessage, lengthOfError);
                        write(connectfd, "\n", 1);
                    }
                    if (getCheck < 0){
                        close(dcConnectfd); 
                        dcConnectfd = -1;
                    }
                }

                // RCVD 'P' FROM CLIENT
                if (commandLetter == 'P'){
                    int putCheck = 0;
                    putCheck = putServerFunction(connectfd, dcConnectfd, buffer);
                    if (putCheck == -1){
                        printf("Eerror performing server put function.\n");
                        char *errorMessage = "Eerror calling the putServerFunction.";
                        int lengthOfError = 0;
                        lengthOfError = strlen(errorMessage);
                        write(connectfd, errorMessage, lengthOfError);
                        write(connectfd, "\n", 1);
                    }
                    if (putCheck < 0){
                        close(dcConnectfd); 
                        dcConnectfd = -1;
                    }
                }

                // RCVD 'Q' FROM CLIENT
                if (commandLetter == 'Q'){
                    write(connectfd, "A\n\0", 2);
                    close(connectfd);
                    exit(0);
                }
                
                continue; 
            }

        }

    }

}

// "D" DATA CONNECTION FUNCTION BODY
int dataConnectionServerFunction(int connectfd){
    int dcSetSockOptCheck = 0;
    int listenCheck = 0;
    int dclistenfd = 0;
    int dcConnectfd;
    unsigned int dcLength;
    char dcHostName[NI_MAXHOST];
    int dcBindCheck = 0;
    int dcGetSockNameCheck = 0;
    int dcListenCheck = 0;
    int dcHostEntry = 0;
    int bytesRead = 0; 
    int portNumberAssignedByBind;
    char portAsString[10];

    struct sockaddr_in dcServAddr;
    struct sockaddr_in dcClientAddr; 

    memset(&dcServAddr, 0, sizeof(dcServAddr));
    dcServAddr.sin_family = AF_INET;
    dcServAddr.sin_port = htons(0);
    dcServAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (DEBUG) printf("Command was a 'D', now I'm going to set up a data connection for you.\n");
    fflush(stdout);
                    
    // CREATE A SOCKET FOR DATA CONNECTION AND ERROR CHECK
    dclistenfd = socket(AF_INET, SOCK_STREAM, 0); // Pg. 1155 in textbook.  The socket system call "installs a telephone".
                    
    if (dclistenfd == -1){ // Returns -1 on error. 
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-1);
    } 

    // INITIALIZE SOCKET AND ERROR CHECK
    dcSetSockOptCheck = setsockopt(dclistenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // This will allow you to immediately reuse this port, very important to do this
    if (dcSetSockOptCheck == -1){
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-1); 
    }

    // BIND AND ERROR CHECK
    dcBindCheck = bind(dclistenfd, (struct sockaddr *) &dcServAddr, sizeof(dcServAddr));
    if (dcBindCheck < 0){
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-1);
    }

    // GET LENGTH FOR ACCEPT FUNCTION (NEXT STEP)
    dcLength = sizeof(struct sockaddr_in); // Get length to pass into accept function

    // GETSOCKNAME
    dcGetSockNameCheck = getsockname(dclistenfd, (struct sockaddr *) &dcServAddr, &dcLength);
    if (dcGetSockNameCheck != 0){
        fprintf (stderr, "E%s\n", strerror(errno));
        return (-1);
    }

    portNumberAssignedByBind = ntohs(dcServAddr.sin_port); // Storing port number into an int using ntohs for correct byte order
    if (DEBUG) printf("Port number assigned is: %d\n", portNumberAssignedByBind);
    sprintf(portAsString, "%d", portNumberAssignedByBind);
    if (DEBUG) printf("Port number as a string: %s\n", portAsString);
    int lengthofPortNum = strlen(portAsString);
    if (DEBUG) printf("Length of port number string is: %d\n", lengthofPortNum);

    char acknowledgementWithPortNumber[BUFFER_SIZE] = "A";
    char newline[2] = "\n";
    strcat(acknowledgementWithPortNumber, portAsString);
    strcat(acknowledgementWithPortNumber, newline);
    if (DEBUG) printf("<%s>\n", acknowledgementWithPortNumber);

    int writeCheck = write(connectfd, acknowledgementWithPortNumber, strlen(acknowledgementWithPortNumber));
    if (DEBUG) printf("Value of writecheck is: %d\n", writeCheck);

    // CALL LISTEN
    dcListenCheck = listen(dclistenfd, 4); // Takes 4 attempts before blocking. 
    if (DEBUG) printf("Value of listen: %d\n", dcListenCheck);
    if (dcListenCheck == -1){
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-2);
    }

    // CALL ACCEPT
    if (DEBUG) printf("About to call accept.\n");
    dcConnectfd = accept(dclistenfd, (struct sockaddr *) &dcClientAddr, &dcLength); // On success, returns a file descriptor
    if (DEBUG) printf("Value of accept: %d\n", dcConnectfd);
    if (dcConnectfd == -1){
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-2);
    }

    if (DEBUG) printf("File descriptor returned by accept: %d\n", dcConnectfd);
    if (DEBUG) printf("Data connection has been established.\n");

    return (dcConnectfd);
}

// RLS SERVER FUNCTION BODY (JUST THE PART AFTER RECEIVING L)
int rlsServerFunction(int connectfd, int dcConnectfd){
    if (DEBUG) printf("DO RLS L STUFF HERE!\n");

    // CHECK THAT DATA CONNECTION EXISTS
    if (dcConnectfd < 0){
        printf("Data connection actually wasn't established.\n");
        return (-1);
    }

    // SEND ACKNOWLEGEMENT TO CLIENT
    write(connectfd, "A\n", 2);

    int forkCheck = 0;
    int execCheck = 0;
    int waitStatus = 0;
    char *lsCommand;
    lsCommand = "ls";
    char *lsCommands[] = {"ls", "-l", (char *)0};

    forkCheck = fork();
    if (forkCheck == -1){
        fprintf (stderr, "E%s\n", strerror(errno));
        return(-2);
    }

    // IF CHILD, DUP + EXECVP LS
    if (forkCheck == 0){
        dup2(dcConnectfd, 1);
                    
        execCheck = execvp(lsCommand, lsCommands);
        if (execCheck == -1){
            fprintf (stderr, "E%s\n", strerror(errno));
            return (-2);
        }
    }

    // PARENT WAITS
    if (forkCheck != 0){
        wait(&waitStatus);
        if (DEBUG) printf("Closing data connection.\n");
        close(dcConnectfd); // CLOSE DATA CONNECTION
        if (DEBUG) printf("Data connection closed.\n");
        dcConnectfd = -1; // RESET dcConnectfd after it's done closing
        if (DEBUG) printf("dcConnectfd reset to -1.\n");
    }

    return (0);
}

// "C" PART OF RCD 
int rcdServerFunction(int connectfd, char *buffer){
    char *path;
    path = buffer + 1;
    if (DEBUG) printf("Path is: <%s>\n", path);

    int accessCheck = 0;
    int lengthOfPath = 0;
    struct stat area, *s = &area;
    char *currentWorkingDirectory;

    accessCheck = access(path, R_OK | X_OK); // Check execute priveleges
    if (accessCheck != 0){ // access returns 0 on success
        fprintf (stderr, "E%s\n", strerror(errno)); // Did not have execute privileges
        return (-1);
    }

    if (lstat(path, s) == 0){ // Examine path
        if (S_ISDIR (s->st_mode)){
            chdir(path);
            write(connectfd, "A\n", 2);
            currentWorkingDirectory = getcwd(path, BUFFER_SIZE);
            if (DEBUG) printf("Server changed directories.\n");
            if (DEBUG) printf("Current server working directory is: %s\n", currentWorkingDirectory);
            return(0);
        }
        else{
            printf("ESomething other than a directory was passed in as path.\n");
            return (-1);
        }
    }
    return (-1);
}

// GET SERVER FUNCTION BODY
int getServerFunction(int dcConnectfd, char *buffer, int connectfd){
    // CHECK THAT DATA CONNECTION EXISTS
    if (dcConnectfd == -1){
        printf("EData connection actually wasn't established.\n");
        return (-1);
    }

    char *path;
    path = buffer + 1;
    if (DEBUG) printf("Path is: <%s>\n", path);

    int accessCheck = 0;
    int openfd = 0;
    int currentOffset = 0;
    int bytesRead = 0;
    struct stat area, *s = &area;
    char *currentWorkingDirectory;

    accessCheck = access(path, F_OK); // Check if file exists
    if (accessCheck != 0){ // access returns 0 on success
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-1);
    }

    accessCheck = access(path, R_OK); // Check read priveleges
    if (accessCheck != 0){ // access returns 0 on success
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (-1);
    }

    if (lstat(path, s) == 0){ // Examine path
        if (S_ISREG (s->st_mode)){ // If what was passed in to readable function was a reg. file
            // SEND ACKNOWLEGEMENT TO CLIENT
            if (DEBUG) printf("The file is regular and readable, I'm going to send over your A.\n");
            write(connectfd, "A\n\0", 2);
            openfd = open(path, O_RDONLY); // open the file, returns a file descriptor.
            if (openfd == -1){
                fprintf (stderr, "E%s\n", strerror(errno)); 
                return (-2);
            }
            if (DEBUG) printf("Opened the file.  File descr is: %d\n", openfd);
            currentOffset = lseek(openfd, 0, SEEK_SET); // Move file offset to beginning of file
            if(currentOffset == -1){ // Error check after using LSEEK
                fprintf(stderr, "E%s\n", strerror(errno));
                return (-2);
            }
            if (DEBUG) printf("Preparing to read the file and write it over to client.\n");
            // READ FROM THE FILE, WRITE THOSE BYTES TO DATA CONNECTION
            while ((bytesRead = read(openfd, buffer, BUFFER_SIZE)) > 0){
                write(dcConnectfd, buffer, bytesRead);
            }

            //CLOSE FILE
            printf("File write complete.  Closing the file.\n");
            close(openfd);
            printf("File closed.\n");
            // CLOSE DATA CONNECTION
            close(dcConnectfd); 
            printf("Data connection closed.\n");
            dcConnectfd = -1;
            return (0);
        }
        return (-1);
    }

    else{
        printf("Esomething other than regular file passed in as path.\n");
        close(dcConnectfd); 
        dcConnectfd = -1;
        return (-1);
    } 
}

// PUT SERVER FUNCTION PROTOTYPE
int putServerFunction(int connectfd, int dcConnectfd, char *buffer){
    int readCheck = 0;
    int accessCheck = 0;
    int bytesRead = 0;

    if (DEBUG) printf("I read in your P.\n");

    // CHECK THAT DATA CONNECTION EXISTS
    if (dcConnectfd == -1){
        printf("EData connection actually wasn't established.\n");
        return (-1);
    }

    printf("Here's what the client sent me: <%s>\n", buffer);
    char *path;
    path = buffer + 1;
    if (DEBUG) printf("Path (filename, really) passed in by client is: <%s>\n", path);

    accessCheck = access(path, F_OK); // Check if file already exists
    if (DEBUG) printf("Access returned: %d\n", accessCheck);
    if (accessCheck == 0){
        printf("Ethe file already exists.\n");//The file already exists
        return(-1);
    }

    printf("The file didn't already exist.  I'm going to send you an A.\n");
    // send A to client, file didn't already exist, we good!
    write(connectfd, "A\n\0", 2);

    if (DEBUG) printf("File to create is: <%s>\n", path);
    int openfd = open(path, O_RDWR | O_CREAT, 0644); 
    if (DEBUG) printf("File was created, openfd is: %d\n", openfd);
    if (DEBUG) printf("Getting new file right now...\n");

    // READ FROM THE FILE, WRITE THOSE BYTES TO DATA CONNECTION
    while ((bytesRead = read(dcConnectfd, buffer, BUFFER_SIZE)) > 0){
        if (DEBUG) printf("bytes read: %d\n", bytesRead);
        write(openfd, buffer, bytesRead);
    }

    if (DEBUG) printf("New file write complete.  Going to close the file.\n");
    //CLOSE FILE
    close(openfd);
    if (DEBUG) printf("File closed.\n");
    //CLOSE + RESET DATA CONNECTION
    close(dcConnectfd); 
    dcConnectfd = -1;
    if (DEBUG) printf("Data connection closed and reset to -1.\n");
    return (0);
}


