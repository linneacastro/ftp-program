// CLIENT FILE: myftp.c

/* An FTP CLIENT Program Written in C */
// NAME: Linnea P. Castro
// DATE: 10 NOV 2023
// COURSE: CS 360
// ASSIGNMENT: Final Project

// LIBRARIES TO INCLUDE
#include "myftp.h"

/*
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
*/

// MAIN FUNCTION BODY
int main(int argc, char const *argv[]){

    // VARIABLES
    int listenfd = 0;
    int setSockOptCheck = 0;

    // CREATE A SOCKET AND ERROR CHECK
    listenfd = socket(AF_INET, SOCK_STREAM, 0); // Pg. 1155 in textbook.  The socket system call "installs a telephone".
    if (listenfd == -1){ // Returns -1 on error. 
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (errno);
    } 

    // INITIALIZE SOCKET AND ERROR CHECK
    setSockOptCheck = setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, &(int){1}, sizeof(int)); // This will allow you to immediately reuse this port, very important to do this
    if (setSockOptCheck == -1){
        fprintf (stderr, "E%s\n", strerror(errno)); 
        return (errno); 
    }

    // CHECKING COMMAND LINE ARGS 
    if (argc == 1){ // If user only specified program name, no file given
        printf("Error: Insufficient command line arguments.\n");
        return (0);
    }

    // IF CLIENT
    if (argc == 2){ // 2 command line args given
            clientFunction((char *)argv[1]); // Call clientFunction, passing in argv[1] as address
    }

    // TOO MANY COMMAND LINE ARGS
    if (argc > 2){ // If more than 2 command line args are given
        printf("Etoo many command line arguments.\n");
        return (0);
    }

    return (0);
}

//CLIENT FUNCTION BODY
void clientFunction(char *address){
    int connectCheck = 0;
    int readCheck = 0;
    int err;

    int socketfd;
    int dcsocketfd;
    struct addrinfo hints, *actualdata;
    memset(&hints, 0, sizeof(hints));

    hints.ai_socktype = SOCK_STREAM;
    hints.ai_family = AF_INET;

    char buffer[BUFFER_SIZE] = {0};
    char fileBuffer[BUFFER_SIZE] = {0};
    int bytesRead = 0;
    char *token; 
    token = "";
    int tokenCount = 0;
    char *firstToken;
    char *secondToken;

    int secondTokenFlag;
    secondTokenFlag = 0;

    // SET UP ADDRESS OF SERVER AND ERROR CHECK
    err = getaddrinfo(address, MY_PORT_NUMBER_CLIENT, &hints, &actualdata); 
    if (err != 0){
        fprintf (stderr, "E%s\n", gai_strerror(err)); 
        exit(1);
    }

    // SET UP SOCKET AFTER GETADDRINFO
    socketfd = socket(actualdata->ai_family, actualdata->ai_socktype, 0);
    if (socketfd == -1){ // Returns -1 on error. 
        fprintf (stderr, "E%s\n", strerror(errno)); 
        exit(1);
    } 

    // ATTEMPT TO CONNECT AND ERROR CHECK
    connectCheck = connect(socketfd, actualdata->ai_addr, actualdata->ai_addrlen); // Slide 14
    if (connectCheck == -1){
        fprintf (stderr, "E%s\n", strerror(errno)); 
        exit(1);
    }

    // PRINT CONNECTION MESSAGE IF SUCCESSFUL
    printf("Connected to server %s\n", (char *)address); 

    while(1){
    // RECEIVE INPUT FROM USER VIA STDIN

    int i = 0;
    while ((readCheck = read(0, &buffer[i], 1) > 0)){
        if (buffer[i] == '\n'){
            break;
        }
        i++;
    }
    if (DEBUG) printf("Value of readcheck: %d\n", readCheck);
    if (readCheck == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        exit(1); 
    }

    buffer[i+1] = '\0'; 

    if (DEBUG) printf("<%s>\n", buffer);
    firstToken = strtok(buffer, " "); // TOKEN #1 Separate or "tokenize" buffer with spaces
    if (DEBUG) printf("first Token is: %s\n", firstToken);
    tokenCount++;
    if (DEBUG) printf("token before while loop: <%s>\n", token);

    while (token != NULL){
        token = strtok(NULL, " ");
        if (secondTokenFlag == 0){
            secondToken = token;
            secondTokenFlag = 1;
        }
        if (DEBUG) printf("Token is: %s\n", token);
        tokenCount++;
    }

    tokenCount = tokenCount - 1; // Adjust tokenCount
    if (DEBUG) printf("Number of tokens: %d\n", tokenCount);
    if (DEBUG) printf("First token is: <%s>\n", firstToken);
    if (DEBUG) printf("Second token is: <%s>\n", secondToken); 

    if (tokenCount > 2 ){
        printf("E: Too many tokens entered.  Please do not exceed 2 tokens.\n");
        tokenCount = 0; // RESET 
        bytesRead = 0; // RESET
        readCheck = 0; // RESET
        secondTokenFlag = 0; // RESET
        token = ""; // Make token not empty, so it will enter into while (token != NULL) loop
        fflush(stdout); // Not sure if this is necessary?
        for (int i = 0 ; i < BUFFER_SIZE ; i++){
            buffer[i] = '\0'; // can also use memset to do this more efficiently
            //resetFunction();
        }
        continue;
    }

    // LS WITH EXTRA TOKENS
    if (strcmp(firstToken, "ls") == 0 ){
        printf("E: ls should only be used as a single token\n");
        tokenCount = 0; // RESET 
        bytesRead = 0; // RESET
        readCheck = 0; // RESET
        secondTokenFlag = 0; // RESET
        token = ""; // Make token not empty, so it will enter into while (token != NULL) loop
        fflush(stdout); 
        for (int i = 0 ; i < BUFFER_SIZE ; i++){
            buffer[i] = '\0'; // can also use memset to do this more efficiently
        }
        continue;
    }
    
    // LS SINGLE TOKEN, LEGIT
    if (strcmp(firstToken, "ls\n") == 0){
        int lsResult = 0;
        lsResult = lsFunction();

        if (lsResult == -1){
            printf("Eerror calling ls.\n");
        }

    }

    // CD NO DIRECTORY PROVIDED
    if (strcmp(firstToken, "cd\n") == 0 ){
        printf("Ecd is not a standalone command.  Please follow cd with <pathname>\n");
    }

    // CD WITH PATH (2ND TOKEN), LEGIT
    if (strcmp(firstToken, "cd") == 0){
        int cdResult = 0;
        cdResult = cdFunction(secondToken);

        if (cdResult == -1){
            printf("Eerror calling cd.\n"); 
        }
    }

    // RLS WITH MORE TOKENS, NOT CORRECT
    if (strcmp(firstToken, "rls") == 0){
        printf("E: rls should only be used as a single token\n");
    }
    
    // RLS AS FIRST TOKEN, NONE FOLLOWING, LEGIT
    if (strcmp(firstToken, "rls\n") == 0){
        dcsocketfd = dataConnectionClientFunction(socketfd, buffer, address);
        if (DEBUG) printf("Returned from the data connection creation function, again, dcsockedfd is: %d\n", dcsocketfd);
        if (DEBUG) printf("Now going to call the next part of RLS\n");


        if (dcsocketfd != -1){ // don't call rls function if dataconn returned -1
            int rlsCheck = 0;
            rlsCheck = (rlsClientFunction(socketfd, dcsocketfd, buffer));
            if (rlsCheck == -1){
                printf("Eerror calling rls.\n"); 
            }
        }
    }

    // RCD BY ITSELF, NOT LEGIT
    if (strcmp(firstToken, "rcd\n") == 0 ){
        printf("E: rls should only be used as a single token\n");
    }


    // RCD WITH PATHNAME, LEGIT
    if (strcmp(firstToken, "rcd") == 0){
        int rcdCheck = 0;
        rcdCheck = rcdClientFunction(socketfd, secondToken, buffer);
        if (rcdCheck == -1){
            printf("Eerror calling rcd.\n");    
        }
    }

    // GET NO PATH PROVIDED
    if (strcmp(firstToken, "get\n") == 0 ){
        printf("E: get is not a standalone command.  Please follow get with <path/filename>\n");
    }

    // GET WITH PATH, LEGIT
    if (strcmp(firstToken, "get") == 0 ){
        int getCheck = 0;
        getCheck = getClientFunction(secondToken, buffer, address, socketfd);
        if (getCheck == -1){
            printf("Eerror calling get.\n");
        }
    }

    // SHOW NO PATH PROVIDED
    if (strcmp(firstToken, "show\n") == 0 ){
        printf("Eshow is not a standalone command.  Please follow show with <pathname>.\n");
    }

    // SHOW WITH PATH, LEGIT
    if (strcmp(firstToken, "show") == 0 ){
        int showCheck = 0;
        showCheck = showClientFunction(secondToken, buffer, address, socketfd);
        if (showCheck == -1){
            printf("Eerror calling show.\n");
        }
    }

    // PUT NO PATH PROVIDED
    if (strcmp(firstToken, "put\n") == 0 ){
        printf("Eput is not a standalone command.  Please follow put with <pathname>.\n");
    }

    // PUT WITH PATH PROVIDED, LEGIT
    if (strcmp(firstToken, "put") == 0 ){
        int putCheck = 0;
        putCheck = putClientFunction(socketfd, buffer, address, secondToken);
        if (putCheck == -1){
            printf("Eerror calling put.\n");
        }
    }

    // EXIT
    if (strcmp(firstToken, "exit\n") == 0 ){
        int quitCheck = 0;
        quitCheck = quitClientFunction(socketfd, buffer);
        if (quitCheck == -1){
            printf("Eerror calling quit.\n");
        }
    }

    tokenCount = 0; // RESET 
    bytesRead = 0; // RESET
    readCheck = 0; // RESET
    secondTokenFlag = 0; // RESET
    token = ""; // Make token not empty, so it will enter into while (token != NULL) loop
    fflush(stdout); // Not sure if this is necessary?

    for (int i = 0 ; i < BUFFER_SIZE ; i++){
        buffer[i] = '\0'; // can also use memset to do this more efficiently
    }

    continue;
    
    }
}

// LS FUNCTION BODY
int lsFunction(void){
    char *lsCommand;
    lsCommand = "ls";
    char *moreCommand;
    moreCommand = "more";
    char *lsCommands[] = {"ls", "-l", (char *)0};
    char *moreCommands[] = {"more", "-20", (char *)0};
    int fd[2];
    int pipeError = 0;
    int forkStatus = 0;
    int execError = 0;
    int waitStatus = 0;

    // CALL PIPE AND CHECK FOR ERROR
    pipeError = pipe(fd);
    if (pipeError == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1);
    }

    // CALL FORK AND CHECK FOR ERROR
    forkStatus = fork();
    if (forkStatus == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1);
    }

    // CHILD PROCESS CALLS LS
    if (forkStatus == 0){ 
        dup2(fd[1], 1);
        close(fd[1]);
        close(fd[0]); // close READ end of pipe

        execError = execvp(lsCommand, lsCommands);
        if (execError == -1){
            fprintf (stdout, "E%s\n", strerror(errno));
            return (-1);
        }
    }

    // PARENT CALLS FORK 
    if (forkStatus != 0){

        // CALL FORK AGAIN AND CHECK FOR ERROR
        forkStatus = fork();
        if (forkStatus == -1){
            fprintf (stdout, "E%s\n", strerror(errno));
            return (-1);
        }

        // PARENT WAITS
        if (forkStatus != 0){
            close(fd[0]);
            close(fd[1]);
            wait(&waitStatus);
        }
            
        // CHILD OF PARENT IS CALLING MORE
        if (forkStatus == 0){
            dup2(fd[0], 0);
            close(fd[0]);
            close(fd[1]); // close WRITE end of pipe
            execError = execvp(moreCommand, moreCommands);
            if (execError == -1){
                fprintf (stdout, "E%s\n", strerror(errno));
                return (-1);
            }
        }

        wait(&waitStatus);
            
    }
    return (0);
}

// CD FUNCTION BODY
int cdFunction(char *secondToken){
        int accessCheck = 0;
        int lengthOfSecondToken = 0;
        struct stat area, *s = &area;
        char *currentWorkingDirectory;

        if (DEBUG) printf("cd was inputted\n");
        lengthOfSecondToken = strlen(secondToken);
        if (DEBUG) printf("Second token is: <%s>", secondToken);
        if (DEBUG) printf("Length of second token is (includes newline): %d\n", lengthOfSecondToken);
        secondToken[lengthOfSecondToken - 1] = '\0';
        if (DEBUG) printf("NEW second token is: <%s>\n", secondToken);

        accessCheck = access(secondToken, R_OK | X_OK); // Check execute priveleges
        if (accessCheck != 0){ // access returns 0 on success
            fprintf (stdout, "E%s\n", strerror(errno)); // Did not have execute privileges
            return (-1);
        }

        if (lstat(secondToken, s) == 0){ // Examine what second token passed in
            if (S_ISDIR (s->st_mode)){
                chdir(secondToken);
                currentWorkingDirectory = getcwd(secondToken, BUFFER_SIZE);
                if (DEBUG) printf("We changed directories.\n");
                if (DEBUG) printf("Current working directory is: %s\n", currentWorkingDirectory);
                return(0);
            }
            else{
                printf("Esomething other than a directory was passed in as second token.\n");
                return (-1);
            }
        }
        return (-1);
}

// "D" DATA CONNECTION CLIENT SIDE FUNCTION BODY
int dataConnectionClientFunction(int socketfd, char *buffer, char *address){
    int readCheck = 0;
    char *sendD = "D\n";
    write(socketfd, sendD, 2);
    if (DEBUG) printf("Sent request for data connection to server.\n");
    int i = 0;
    while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
        if (buffer[i] == '\n'){
            break;
        }
        i++;
    }
    if (DEBUG) printf("Value of readcheck: %d\n", readCheck);
    if (readCheck == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1);
    }

    buffer[i] = '\0'; // Take off the newline, replace with null term.
    if (buffer[0] == 'E'){
        return (-1);
    }

    if (DEBUG) printf("What the server just sent me: <%s>\n", buffer);
    char *bufferWithPortNumber = &buffer[1];
    if (DEBUG) printf("<%s>\n", bufferWithPortNumber);
    int lengthOfBufferWithPortNumber = strlen(bufferWithPortNumber);
    if (DEBUG) printf("Length of buffer with portnum: %d\n", lengthOfBufferWithPortNumber);

    int dcConnectCheck = 0;
    int dcReadCheck = 0;
    int dcerr = 0;
    int dcsocketfd;
    struct addrinfo dchints, *actualdata;
    memset(&dchints, 0, sizeof(dchints));

    dchints.ai_socktype = SOCK_STREAM;
    dchints.ai_family = AF_INET;

    // SET UP ADDRESS OF SERVER DATA CONNECTION
    dcerr = getaddrinfo(address, bufferWithPortNumber, &dchints, &actualdata);
    if (dcerr != 0){
        fprintf (stdout, "E%s\n", gai_strerror(dcerr));
        return (-1);
    }

    // SET UP SOCKET FOR DATA CONNECTION
    dcsocketfd = socket(actualdata->ai_family, actualdata->ai_socktype, 0); 
    if (dcsocketfd == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1);
    }  

    dcConnectCheck = connect(dcsocketfd, actualdata->ai_addr, actualdata->ai_addrlen); // Slide 14
    if (dcConnectCheck == -1){
        fprintf (stdout, "E%s\n", strerror(errno)); 
        return(-1);
    }

    if (DEBUG) printf("file descriptor returned by socket: %d\n", dcsocketfd);

    return (dcsocketfd);
}

// RLS "L" PART OF CLIENT FUNCTION BODY
int rlsClientFunction(int socketfd, int dcsocketfd, char *buffer){
    char *sendL = "L\n";
    char *moreCommand;
    moreCommand = "more";
    char *moreCommands[] = {"more", "-20", (char *)0};
    int forkStatus = 0;
    int waitStatus = 0;
    int execError = 0;
    int readCheck = 0;

    write(socketfd, sendL, 2);
    if (DEBUG) printf("Got a data connection. Sent L to server.\n");

    int i = 0;
    while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
        if (buffer[i] == '\n'){
            break;
        }
        i++;
    }
    if (DEBUG) printf("Value of readcheck: %d\n", readCheck);
    if (readCheck == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1);
    }

    buffer[i] = '\0'; // Take off the newline, replace with null term.

    printf("Received: <%s> from server.\n", buffer);
    if (buffer[0] == 'E'){
        return (-1);
    }
    if (buffer[0] == 'A'){
        if (DEBUG) printf("I got your A.\n");
        fflush(stdout);

        // FORK
        forkStatus = fork();
        if (forkStatus == -1){
            fprintf (stdout, "E: %s\n", strerror(errno));
            return (-1);
        }

        // CHILD CALLS MORE
        if (forkStatus == 0){
            dup2(dcsocketfd, 0);

            execError = execvp(moreCommand, moreCommands);
                if (execError == -1){
                    fprintf (stdout, "E: %s\n", strerror(errno));
                    return(-1);
                }
        }

        // PARENT WAITS 
        if (forkStatus != 0){
            wait(&waitStatus);
        }
        if (DEBUG) printf("Closing data connection.\n");
        close(dcsocketfd);
        if (DEBUG) printf("Closed data connection.\n");
        return(0);
    }
    return (-1);
}

// "C" PART OF RCD CLIENT FUNCTION BODY
int rcdClientFunction(int socketfd, char *secondToken, char *buffer){
    int readCheck = 0;
    char cCommandWithPath[BUFFER_SIZE] = "C\0";
    strcat(cCommandWithPath, secondToken);
    if (DEBUG) printf("Concatting 'C' with second token/path plus newline to send to server.\n");
    if (DEBUG) printf("Concatted string: <%s>\n", cCommandWithPath);

    write(socketfd, cCommandWithPath, strlen(cCommandWithPath));

    if (DEBUG) printf("Sent C<pathname> to server.\n");

    int i = 0;
    while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
        if (buffer[i] == '\n'){
            break;
        }
        i++;
    }
    printf("Value of readcheck: %d\n", readCheck);
    if (readCheck == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1);
    }

    buffer[i] = '\0'; // Take off the newline, replace with null term.

    printf("Received: <%s> from server.\n", buffer);
    if (buffer[0] == 'E'){
        return (-1);
    }
    if (buffer[0] == 'A'){
        if (DEBUG) printf("I got your A.\n");
        fflush(stdout);
    }
    return (0);
}

// GET CLIENT SIDE FUNCTION BODY
int getClientFunction(char *secondToken, char *buffer, char *address, int socketfd){
    int readCheck = 0;
    int accessCheck = 0;
    int bytesRead = 0;
    int dcsocketfd;
    if (DEBUG) printf("second token is: <%s>\n", secondToken);
    int lengthOfSecondToken = strlen(secondToken);
    char *fileToCreate;

    char fileToGet[lengthOfSecondToken + 1];
    strcpy(fileToGet, secondToken); // This fixed garbled secondToken!
    if (DEBUG) printf("file to get is: <%s>\n", fileToGet);
    char fileToGetWithoutNewline[lengthOfSecondToken +1];
    strcpy(fileToGetWithoutNewline, secondToken);
    fileToGetWithoutNewline[lengthOfSecondToken - 1] = '\0';
    if (DEBUG) printf("file to get without newline is <%s>\n", fileToGetWithoutNewline);
    char *fileWithNoSlashesOrNewline = strrchr(fileToGetWithoutNewline, '/');

    if (DEBUG) printf("Filename is: <%s>\n", fileWithNoSlashesOrNewline);

    if (fileWithNoSlashesOrNewline == NULL){ // THERE WERE NO SLASHES, use fileToGetWithoutNewline
        accessCheck = access(fileToGetWithoutNewline, F_OK); // Check if file exists
        if (DEBUG) printf("Access returned: %d\n", accessCheck);
        if (accessCheck == 0){
            printf("Ethe file already exists.\n");//The file already exists
            return(-1);
        }
        fileToCreate = fileToGetWithoutNewline;
    }

    else{ // THERE WERE SLASHES, use new pointer, fileWithNoSlashesOrNewline for access check
        fileWithNoSlashesOrNewline++; // increment the pointer to get rid of the slash up front
        accessCheck = access(fileWithNoSlashesOrNewline, F_OK); // Check if file exists
        if (DEBUG) printf("Access returned: %d\n", accessCheck);
        if (DEBUG) printf("Value we evaluated for access: <%s>\n", fileWithNoSlashesOrNewline);
        if (accessCheck == 0){
            printf("Ethe file already exists.\n");//The file already exists
            return (-1);
        }
        fileToCreate = fileWithNoSlashesOrNewline;
    }

    dcsocketfd = dataConnectionClientFunction(socketfd, buffer, address);

    if (dcsocketfd != -1){ // don't call rls function if dataconn returned -1
        if (DEBUG) printf("Returned from the data connection creation function, again, dcsockedfd is: %d\n", dcsocketfd);
        if (DEBUG) printf("Now going to call the next part of GET\n");
        if (DEBUG) printf("file to get is: <%s>\n", fileToGet);

        char gCommandWithPath[BUFFER_SIZE] = "G\0";
        strcat(gCommandWithPath, fileToGet);
        if (DEBUG) printf("Concatting 'G' with second token/path plus newline to send to server.\n");
        if (DEBUG) printf("Concatted string: <%s>\n", gCommandWithPath);
        write(socketfd, gCommandWithPath, strlen(gCommandWithPath));

        if (DEBUG) printf("Sent G<pathname> to server.\n");
        
        int i = 0;
        while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
            if (buffer[i] == '\n'){
                break;
            }
            i++;
        }
        if (DEBUG) printf("Value of readcheck: %d\n", readCheck);
        if (readCheck == -1){
            fprintf (stdout, "E%s\n", strerror(errno));
            return (-1); 
        }

        buffer[i] = '\0'; // Take off the newline, replace with null term.

        printf("Received: <%s> from server.\n", buffer);
        if (buffer[0] == 'E'){
            return (-1);
        }
        if (buffer[0] == 'A'){
            if (DEBUG) printf("I got your A.\n");
            fflush(stdout);
        }

        if (DEBUG) printf("File to create is: <%s>\n", fileToCreate);
        int openfd = open(fileToCreate, O_RDWR | O_CREAT, 0644); 
        if (DEBUG) printf("File was created, openfd is: %d\n", openfd);
        if (DEBUG) printf("Getting new file right now...\n");

        // READ FROM THE FILE, WRITE THOSE BYTES TO DATA CONNECTION
        while ((bytesRead = read(dcsocketfd, buffer, BUFFER_SIZE)) > 0){
            write(openfd, buffer, bytesRead);
        }

        if (DEBUG) printf("New file write complete.  Going to close the file.\n");
        //CLOSE FILE
        close(openfd);
        if (DEBUG) printf("File closed.\n");
        //CLOSE DATA CONN.
        close(dcsocketfd);
        return (0);
    }
    return (-1);
}

// SHOW CLIENT SIDE FUNCTION PROTOTYPE
int showClientFunction(char *secondToken, char *buffer, char *address, int socketfd){
    int readCheck = 0;
    int accessCheck = 0;
    int bytesRead = 0;
    int dcsocketfd;
    int forkStatus = 0;
    int execError = 0;
    char *moreCommand;
    moreCommand = "more";
    char *moreCommands[] = {"more", "-20", (char *)0};
    int waitStatus = 0;

    if (DEBUG) printf("second token is: <%s>\n", secondToken);
    char gCommandWithPath[BUFFER_SIZE] = "G\0";
    strcat(gCommandWithPath, secondToken);

    dcsocketfd = dataConnectionClientFunction(socketfd, buffer, address);
    if (dcsocketfd != -1){ // don't call rls function if dataconn returned -1
        if (DEBUG) printf("Returned from the data connection creation function, again, dcsockedfd is: %d\n", dcsocketfd);
        if (DEBUG) printf("Now going to do the next thing for show... which is sending a G to server.\n");
        
        if (DEBUG) printf("Sending this to server <%s>\n", gCommandWithPath);
        write(socketfd, gCommandWithPath, strlen(gCommandWithPath));
        if (DEBUG) printf("Sent G<pathname> to server.\n");

        if (DEBUG) printf("Now to do the fork and exec more\n");

        int i = 0;
        while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
            if (buffer[i] == '\n'){
                break;
            }
            i++;
        }
        printf("Value of readcheck: %d\n", readCheck);
        if (readCheck == -1){
            fprintf (stdout, "E%s\n", strerror(errno));
            return (-1);
        }

        buffer[i] = '\0'; // Take off the newline, replace with null term.

        printf("Received: <%s> from server.\n", buffer);
        if (buffer[0] == 'E'){
            return (-1);
        }
        if (buffer[0] == 'A'){
            if (DEBUG) printf("I got your A.\n");
            fflush(stdout);

            // FORK
            forkStatus = fork();
            if (forkStatus == -1){
                fprintf (stdout, "E: %s\n", strerror(errno));
                return (-1);
            }

            // CHILD CALLS MORE
            if (forkStatus == 0){
                dup2(dcsocketfd, 0);

                execError = execvp(moreCommand, moreCommands);
                    if (execError == -1){
                        fprintf (stdout, "E: %s\n", strerror(errno));
                        return(-1);
                    }
            }

            // PARENT WAITS 
            if (forkStatus != 0){
                wait(&waitStatus);
            }
            if (DEBUG) printf("Closing data connection.\n");
            close(dcsocketfd);
            if (DEBUG) printf("Closed data connection.\n");
            return(0);
        }
    }
    return (-1);
}

// PUT CLIENT FUNCTION PROTOTYPE
int putClientFunction(int socketfd, char *buffer, char *address, char *secondToken){
    int accessCheck = 0;
    int readCheck = 0;
    int dcsocketfd = 0;
    int openfd = 0;
    int currentOffset = 0;
    int bytesRead = 0;
    int offset = 0;
    struct stat area, *s = &area;

    if (DEBUG) printf("Path/file to put is secondToken, which is: <%s>\n", secondToken);
    int lengthOfSecondToken = strlen(secondToken);
    if (DEBUG) printf("length of secondToken is: %d\n", lengthOfSecondToken);
    char fileToPut[lengthOfSecondToken + 1];
    strcpy(fileToPut, secondToken); // This fixed garbled secondToken after calling data conn. function
    if (DEBUG) printf("File to put is (want to keep newline): <%s>\n", fileToPut);
    secondToken[lengthOfSecondToken - 1] = '\0'; // replace newline with null term.
    if (DEBUG) printf("secondToken without newline is: <%s>\n", secondToken);

    accessCheck = access(secondToken, R_OK); // Check read priveleges
    if (accessCheck != 0){ // access returns 0 on success
        fprintf (stdout, "E%s\n", strerror(errno)); // Did not have read privileges
        return (-1);
    }

    if (lstat(secondToken, s) == 0){ // Examine path
        if (S_ISREG (s->st_mode)){ // If what was passed in to readable function was a reg. file
            // SEND REQUEST FOR DATA CONN TO SERVER
            if (DEBUG) printf("The file is regular and readable.  I'm going to request a data connection.\n");
        }
        else{
            printf("Efile not a regular file");
            return (-1);
        }
    }

    dcsocketfd = dataConnectionClientFunction(socketfd, buffer, address);
    if (dcsocketfd != -1){ // don't call rls function if dataconn returned -1
        if (DEBUG) printf("dcsocketfd returned: %d\n", dcsocketfd);
        if (DEBUG) printf("value of fileToPut: <%s>\n", fileToPut);

        if (DEBUG) printf("Now I need to parse out last bit of filename to get rid of slashes, if necessary.\n");

        char *fileForServerToCreate;
        char *fileWithNoSlashes = strrchr(fileToPut, '/');
        if (DEBUG) printf("strrchr returned: <%s>\n", fileWithNoSlashes);

        if (fileWithNoSlashes == NULL){ // THERE WERE NO SLASHES, use path to check if file exists
            fileForServerToCreate = fileToPut;
        }

        else{ // there were slashes.  use strrchr return with pointer incremented once to get rid of upfront slash
            fileWithNoSlashes++; // increment the pointer to get rid of the slash up front
            fileForServerToCreate = fileWithNoSlashes;
        }

        if (DEBUG) printf("Now going to send P<pathname> over to server.\n");
        char pCommandWithPath[BUFFER_SIZE] = "P\0";
        strcat(pCommandWithPath, fileForServerToCreate);
        if (DEBUG) printf("Concatting 'P' with filename plus newline to send to server.\n");
        if (DEBUG) printf("Concatted string: <%s>\n", pCommandWithPath);
        write(socketfd, pCommandWithPath, strlen(pCommandWithPath));
        if (DEBUG) printf("Sent P<filename> to server.\n");
            
        int i = 0;
        while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
            if (buffer[i] == '\n'){
                break;
            }
            i++;
        }
        printf("Value of readcheck: %d\n", readCheck);
        if (readCheck == -1){
            fprintf (stdout, "E%s\n", strerror(errno));
            return (-1); 
        }

        printf("Received: <%s> from server.\n", buffer);
        if (buffer[0] == 'E'){
            return (-1);
        }
        if (buffer[0] == 'A'){
            if (DEBUG) printf("I got your A.\n");
            fflush(stdout);
        }

        if (DEBUG) printf("File I'm going to open is: <%s>\n", fileToPut);
        int lengthOfFileToPut = strlen(fileToPut);
        fileToPut[lengthOfFileToPut - 1] = '\0';
        if (DEBUG) printf("File to put is: <%s>\n", fileToPut);
            
        openfd = open(fileToPut, O_RDONLY); // open the file, returns a file descriptor.
        if (openfd == -1){
            fprintf (stdout, "E%s\n", strerror(errno)); 
            return (-1);
        }
        if (DEBUG) printf("Opened the file.  File descr is: %d\n", openfd);
        offset = lseek(openfd, 0, SEEK_SET); // Move file offset to beginning of file
        if(offset == -1){ // Error check after using LSEEK
            fprintf(stdout, "E%s\n", strerror(errno));
            return (-1);
        }

        if (DEBUG) printf("Preparing to read the file and write it over to client.\n");

        // READ FROM THE FILE, WRITE THOSE BYTES TO DATA CONNECTION
        while ((bytesRead = read(openfd, buffer, BUFFER_SIZE)) > 0){
            write(dcsocketfd, buffer, bytesRead);
        }

        //CLOSE FILE
        if (DEBUG) printf("File write complete.  Closing the file.\n");
        close(openfd);
        if (DEBUG) printf("File closed.\n");
        //CLOSE DATA CONNECTION
        close(dcsocketfd);
        if (DEBUG) printf("Data connection closed.\n");
        return (0);
    }
    return (-1);
}

// QUIT CLIENT FUNCTION BODY
int quitClientFunction(int socketfd, char *buffer){
    int readCheck = 0;
    char *sendQ = "Q\n\0";
    write(socketfd, sendQ, 2);
    int i = 0;
    while ((readCheck = read(socketfd, &buffer[i], 1) > 0)){
        if (buffer[i] == '\n'){
            break;
        }
        i++;
    }
    if (DEBUG) printf("Value of readcheck: %d\n", readCheck);
    if (readCheck == -1){
        fprintf (stdout, "E%s\n", strerror(errno));
        return (-1); 
    }

    printf("Received: <%s> from server.\n", buffer);
    if (buffer[0] == 'A'){
        if (DEBUG) printf("I got your A.\n");
        fflush(stdout);
        close(socketfd);
        exit(0);
    }
    return (0);
}

