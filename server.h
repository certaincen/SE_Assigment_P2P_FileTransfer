#include <stdio.h>
#include <map>
#include <iostream>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include <errno.h>


#define MAX_MSG_SIZE 1024
#define SERVER_PORT 1234
#define STATE_PORT 4321
#define ONLINE 1
#define OFFLINE -1
#define BUSY 0
#define Q_ONLINE 1
#define Q_OFFLINE 2
#define Q_BUSY 3
#define Q_QUERY 4
#define SUCCESS 200
#define FAILED 500
#define DEBUG 1

struct client_inform
{
    int port;
    int status;
};
