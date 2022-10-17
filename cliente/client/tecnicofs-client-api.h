#ifndef API_H
#define API_H

#include "tecnicofs-api-constants.h"
#include <sys/socket.h>
#include <sys/un.h>

#define CLIESOCKET "/tmp/client"
#define BASECLIENTSIZE 12

#define SUCCESS 0
#define FAIL -1


int tfsCreate(char *path, char nodeType);
int tfsDelete(char *path);
int tfsLookup(char *path);
int tfsMove(char *from, char *to);
int tfsPrint(char *outputfile);

int tfsMount(char* serverName);
int tfsUnmount();

void setClientName();
int setSockAddrUn(char *path, struct sockaddr_un * addr);

#endif /* CLIENT_H */
