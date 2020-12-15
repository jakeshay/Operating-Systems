#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <unistd.h>
#include <netinet/in.h>
#include <sys/time.h>

#include "queue.h"
#include "util.h"

#define MAX_INPUT_FILES 10

typedef struct Files {
	char *fileName;
	int length;
	int status;
} files;


const int MAX_RESOLVER_THREADS = 10;
const int MAX_REQUESTER_THREADS = 5;
const int MAX_NAME_LENGTH = 1025;
const int MAX_IP_LENGTH = INET6_ADDRSTRLEN;

void *requesterStartRoutine(void *arg);
void *resolverStartRoutine();


