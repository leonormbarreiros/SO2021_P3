#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <sys/stat.h>
#include "fs/operations.h"
#include "tecnicofs-api-constants.h"

#define MAX_INPUT_SIZE 100

/* number of execution threads */
int numberThreads = 0;

char * serverName;
int sockfd; 
struct sockaddr_un server_addr, client_addr; 
socklen_t addrlen;

/*
 * Creates the threads vector
 * Input:
 *  - numT: number of threads 
 * Returns:
 *  - tid: pointer to the threads vector
 *  - FAIL: otherwise
 */
pthread_t * create_threads_vec(int numT) {
    pthread_t * tid;

    /* create the vector */
    if(!(tid = (pthread_t *) malloc(numberThreads * sizeof(pthread_t)))){
        fprintf(stderr, "Error: No memory allocated for threads.\n");
        exit(EXIT_FAILURE);
    }   

    return tid;
}

/** 
 * Auxiliary function
 */ 
static void displayUsage (const char* appName) {
    printf("Usage: %s num_threads socket_name\n", appName);
    exit(EXIT_FAILURE);
}

/** 
 * Parsing the execution arguments
 */ 
static void parseArgs (long argc, char* const argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Invalid format:\n");
        displayUsage(argv[0]);
    }

    // argv[1] is the number of threads
    if((numberThreads = * argv[1] - '0') < 1){ /* ERROR CASE */
        fprintf(stderr, "Error: Invalid number of threads.\n");
        exit(EXIT_FAILURE);
    }

    // argv[2] is the server socket name
    serverName = (char *)malloc(sizeof(char) * (strlen(argv[2])+1));
    strcpy(serverName, argv[2]);
}

/*
 * Auxiliary funcion for error messages
 */
void errorParse(){
    fprintf(stderr, "Error: command invalid\n");
    exit(EXIT_FAILURE);
}

/*
 * Executes the commands in the commands vector
 */
void applyCommands() {
    while (1){
        
        char command[MAX_INPUT_SIZE];
        int c;

        /* receiving the command to apply */
        addrlen = sizeof(struct sockaddr_un);
        c = recvfrom(sockfd, command, sizeof(command)-1, 0, (struct sockaddr *)&client_addr, &addrlen);
        if (c <= 0) {
            perror("server: recvfrom error");
            exit(EXIT_FAILURE);
        }
        command[c] = '\0'; 

        char token/*, type*/;
        char name[MAX_INPUT_SIZE], sec_argument[MAX_INPUT_SIZE];
        int numTokens = sscanf(command, "%c %s %s", &token, name, sec_argument);
        if (numTokens < 2) {
            fprintf(stderr, "Error: invalid command in Queue\n");
            exit(EXIT_FAILURE);
        }

        int res;
        switch (token) {
            case 'c':
                switch (sec_argument[0]) {
                    case 'f':
                        res = create(name, T_FILE);
                        break;
                    case 'd':
                        res = create(name, T_DIRECTORY);
                        break;
                    default:
                        fprintf(stderr, "Error: invalid node type\n");
                        exit(EXIT_FAILURE);
                }
                break;
            case 'l': 
                res = lookup(name);
                break;
            case 'd':
                res = delete(name);
                break;
            case 'm':
                res = move(name, sec_argument);
                break;
            case 'p':
                res = print(name);
                break;
            default: { /* error */
                fprintf(stderr, "Error: command to apply\n");
                exit(EXIT_FAILURE);
            }
        }
        
        /* sending the result of applying the command */
        if (sendto(sockfd, &res, sizeof(int), 0, (struct sockaddr *)&client_addr, addrlen) < 0) {
            perror("server: sendto error");
            exit(EXIT_FAILURE);
        }
    }
}

/*
 * Auxiliary function for using applyCommands with threads.
 */
void * fnThread(void * arg) {
    applyCommands();
    return NULL;
}


/** 
 * Sets the socket address
 */ 
int setSockAddrUn(char * path, struct sockaddr_un * addr) {
    if (addr == NULL) 
        return 0;
    
    bzero((char *)addr, sizeof(struct sockaddr_un));
    addr->sun_family = AF_UNIX;
    strcpy(addr->sun_path, path);
    
    return SUN_LEN(addr);
}

/** 
 * Creates the socket for the server 
 * Input:
 * Returns: SUCCESS or FAIL
 */
int mount() {
    /* create the unix datagram socket */
    if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
        perror("server: can't open socket");
        return FAIL;
    }

    /* give it its name: set address + bind */
    unlink(serverName);
    addrlen = setSockAddrUn(serverName, &server_addr);
    if (bind(sockfd, (struct sockaddr *) &server_addr, addrlen) < 0) {
        perror("server: bind error");
        return FAIL;
    }
    /* cautionary measure */
    if (chmod(serverName, 00222) == -1) {
        perror("client: can't change permissions of socket");
        return FAIL;
    } 
    return SUCCESS;
}

/**
 * Disassembles the server socket
 */ 
void unmount() {
    close(sockfd);
    unlink(serverName);
}

int main(int argc, char* argv[]) {
    int numT;
    pthread_t * tid;

    parseArgs(argc, argv); 
    
    /* assemble server socket */
    if (mount() == SUCCESS)
      printf("Mounted! (socket = %s)\n", serverName);
    else {
      fprintf(stderr, "Unable to mount socket: %s\n", serverName);
      exit(EXIT_FAILURE);
    }

    tid = create_threads_vec(numberThreads); 

    init_fs();

    /* create the execution threads */
    for (numT = 0; numT < numberThreads; numT++) {
        if (pthread_create(&tid[numT], NULL, fnThread, NULL) != 0) {
            exit(EXIT_FAILURE);
        }
    }

    /* waiting for all the threads to finish */
    for (numT = 0; numT < numberThreads; numT++) {
        if (pthread_join(tid[numT], NULL) != 0) {
            exit(EXIT_FAILURE);
        }
    }

    destroy_fs();
    unmount();

    free(serverName);
    free(tid);

    exit(EXIT_SUCCESS);
}

