#include "tecnicofs-client-api.h"
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/stat.h>
#include <stdio.h>
#include <string.h>

char client_name[MAX_INPUT_SIZE]; /* the client's name */
int sockfd; /* the client socket's file descriptor */
socklen_t servlen, clilen; /* size of server and client sockets */
struct sockaddr_un serv_addr, client_addr; /* address of server and client sockets */

/** 
 * Sends a command corresponding to a create operation for the server to
 execute and receives its output.
 * Input:
 * - filename: the file/directory to be created
 * - nodeType: either a file or a directory
 * Returns: SUCCESS or FAIL
 */
int tfsCreate(char *filename, char nodeType) {
  int res, i;
  char command[MAX_INPUT_SIZE];

  /* "reconstructing" the original command */
  command[0] = 'c';
  command[1] = ' ';
  for (i = 0; i < strlen(filename); i++) {
    command[i+2] = filename[i];
  }
  command[i+2] = ' ';
  command[i+2+1] = nodeType;
  command[i+2+2] = '\0';
  
  /* send the command to the server, to be executed */
  if (sendto(sockfd, command, strlen(command) + 1, 0,(struct sockaddr *)&serv_addr, servlen) < 0) {
    perror("client: sendto error");
    return -1;
  }

  /* receive the response from the server, after it has executed the command */
  if (recvfrom(sockfd, &res, sizeof(res), 0,0,0) < 0) {
    perror("client: recvfrom error");
    return -1;
  }
  return res;
}

/** 
 * Sends a command corresponding to a delete operation for the server to
 execute and receives its output.
 * Input:
 * - path: the file/directory to be deleted
 * Returns: SUCCESS or FAIL
 */
int tfsDelete(char *path) {
  int res, i;
  char command[MAX_INPUT_SIZE];

  /* "reconstructing" the original command */
  command[0] = 'd';
  command[1] = ' ';
  for (i = 0; i < strlen(path); i++) {
    command[i+2] = path[i];
  }
  command[i+2] = '\0';

  /* send the command to the server, to be executed */
  if (sendto(sockfd, command, strlen(command) + 1, 0,(struct sockaddr *)&serv_addr, servlen) < 0) {
    perror("client: sendto error");
    return -1;
  }

  /* receive the response from the server, after it has executed the command */
  if (recvfrom(sockfd, &res, sizeof(res), 0,0,0) < 0) {
    perror("client: recvfrom error");
    return -1;
  }
  
  return res;
}

/** 
 * Sends a command corresponding to a move operation for the server to
 execute and receives its output.
 * Input:
 * - from: the file/directory to be moved
 * - to: where it is going to be now 
 * Returns: SUCCESS or FAIL
 */
int tfsMove(char *from, char *to) {
  int res, i, j;
  char command[MAX_INPUT_SIZE];

  /* "reconstructing" the original command */
  command[0] = 'm';
  command[1] = ' ';
  for (i = 0; i < strlen(from); i++) {
    command[i+2] = from[i];
  }
  command[i+2] = ' ';
  for (j = 0; j < strlen(to) ; j++) {
    command[i+2+1+j] = to[j];
  }
  command[i+2+1+j] = '\0';

  /* send the command to the server, to be executed */
  if (sendto(sockfd, command, strlen(command) + 1, 0,(struct sockaddr *)&serv_addr, servlen) < 0) {
    perror("client: sendto error");
    return -1;
  }

  /* receive the response from the server, after it has executed the command */
  if (recvfrom(sockfd, &res, sizeof(res), 0,0,0) < 0) {
    perror("client: recvfrom error");
    return -1;
  }
  return res;
}

/** 
 * Sends a command corresponding to a lookup operation for the server to
 execute and receives its output.
 * Input:
 * - path: the file/directory to look for
 * Returns: SUCCESS or FAIL
 */
int tfsLookup(char *path) {
  int res, i;
  char command[MAX_INPUT_SIZE];

  /* "reconstructing" the original command */
  command[0] = 'l';
  command[1] = ' ';
  for (i = 0; i < strlen(path); i++) {
    command[i+2] = path[i];
  }
  command[i+2] = '\0';

  /* send the command to the server, to be executed */
  if (sendto(sockfd, command, strlen(command) + 1, 0,(struct sockaddr *)&serv_addr, servlen) < 0) {
    perror("client: sendto error");
    return -1;
  }

  /* receive the response from the server, after it has executed the command */
  if (recvfrom(sockfd, &res, sizeof(res), 0,0,0) < 0) {
    perror("client: recvfrom error");
    return -1;
  }
  return res;
}

/** 
 * Sends a command corresponding to a print operation for the server to
 execute and receives its output.
 * Input:
 * - outputfile: the path for the file we're writing on
 * Returns: SUCCESS or FAIL
 */
int tfsPrint(char *outputfile) {
  int res, i;
  char command[MAX_INPUT_SIZE];

  /* "reconstructing" the original command */
  command[0] = 'p';
  command[1] = ' ';
  for (i = 0; i < strlen(outputfile); i++) {
    command[i+2] = outputfile[i];
  }
  command[i+2] = '\0';

  /* send the command to the server, to be executed */
  if (sendto(sockfd, command, strlen(command) + 1, 0,(struct sockaddr *)&serv_addr, servlen) < 0) {
    perror("client: sendto error");
    return -1;
  }

  /* receive the response from the server, after it has executed the command */
  if (recvfrom(sockfd, &res, sizeof(res), 0,0,0) < 0) {
    perror("client: recvfrom error");
    return -1;
  }
  return res;
}

/** 
 * Assemble client socket and connect it to server socket
 * Input:
 * - sockPath: the path for the server socket
 * Returns: SUCCESS or FAIL
 */
int tfsMount(char * sockPath) {
  
  /* init unix datagram socket */
  if ((sockfd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0) {
    perror("client: can't open socket");
    return FAIL;
  }
  
  /* give it its name */
  unlink(client_name);
  clilen = setSockAddrUn(client_name, &client_addr);
  if (bind(sockfd, (struct sockaddr *)&client_addr, clilen) < 0) {
    perror("client: bind error");
    return FAIL;
  }
  if (chmod(client_name, 00222) == -1) {
    perror("client: can't change permissions of socket");
    return FAIL;
  } 

  /* know who the server is (associate name with the server socket) */
  servlen = setSockAddrUn(sockPath, &serv_addr);
  return SUCCESS;
}

/** 
 * Disassembles the client socket.
 */
int tfsUnmount() {
  close(sockfd);
  unlink(client_name);
  return 0;
}

/** 
 * Sets the client name: a concatenation of a fixed client path and the client's pid
 */
void setClientName() {
  int pid = (int) getpid();
  strcpy(client_name, CLIESOCKET);

  /* associating a standard name with the process id allows for multiple clients */
  for (int i = strlen(CLIESOCKET); i < MAX_INPUT_SIZE && pid > 0; i++) {
    client_name[i] = pid % 10;
    pid = pid / 10;
  }
}

/** 
 * Sets the socket address
 */
int setSockAddrUn(char *path, struct sockaddr_un * addr) {
   if (addr == NULL)
    return 0;
  
  bzero((char *)addr, sizeof(struct sockaddr_un));
  addr->sun_family  = AF_UNIX;
  strcpy(addr->sun_path, path);
  
  return SUN_LEN(addr);
}

