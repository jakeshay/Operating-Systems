#include "multi-lookup.h"

buffer queue;
int completed = 0;
pthread_mutex_t fileLock, servicedLock, arrayLock, resultsLock;
pthread_cond_t queueFull, queueEmpty;
char *requesterLog, *resolverLog;

void *requesterStartRoutine(void *arg) {
	files *f = (files *) arg;
	/*Input files, output file, and performance.txt*/
	FILE *fp, *output;
	//FILE *perf;
	/*Count files serviced by thread*/
	int filesServiced = 0;
	/*Hold current line ready by current input files*/
	char line[MAX_NAME_LENGTH];
	/*Loop through input files*/
	for (int i = 0; i < f[0].length; i++) {
		/*Lock access while updating status of current file*/
		pthread_mutex_lock(&fileLock);
		if (f[i].status) {
			f[i].status = 0;
			/*Update status to accessed*/
			pthread_mutex_unlock(&fileLock);
			fp = fopen(f[i].fileName, "r");
			/*Return error if bogus input file*/
			if (!fp) {
				fprintf(stderr, "Bogus input file path: %s\n", f[i].fileName);
			}
			else {
				/*Read file line by line*/
				while(fgets(line, MAX_NAME_LENGTH, fp)) {
					/*Remove new line character so DNS will work*/
					strtok(line, "\n");
					/*Lock access while pushing to shared array*/
					pthread_mutex_lock(&arrayLock);
					while (isFull(&queue)) {
						/*Wait til array is no longer full*/
						pthread_cond_wait(&queueFull, &arrayLock);
					}
					/*Push to array*/
					push(strdup(line), &queue);
					/*Signal any waiting resolvers that the queue is not empty*/
					pthread_cond_signal(&queueEmpty);
					/*Release lock*/
					pthread_mutex_unlock(&arrayLock);
				}
			}
			fclose(fp);
			filesServiced++;
		}
		pthread_mutex_unlock(&fileLock);
	}
	/*Acquire lock to shared output text file*/
	pthread_mutex_lock(&servicedLock);
	output = fopen(requesterLog, "ab");
	//perf = fopen("performance.txt", "ab");
	fprintf(output, "Thread %lu serviced %d files.\n", pthread_self(), filesServiced);
	//fprintf(perf, "Thread %lu serviced %d files.\n", pthread_self(), filesServiced);
	fclose(output);
	//fclose(perf);
	pthread_mutex_unlock(&servicedLock);
	return NULL;
}

void *resolverStartRoutine() {
	char *site;
	FILE *output;
	char ip[MAX_IP_LENGTH];
	while (1) {
		/*Lock while accessing shared array*/
		pthread_mutex_lock(&arrayLock);
		while (isEmpty(&queue)) {
			/*If array is empty and requesters are finished begin exit*/
			if (completed) {
				/*release the lock and terminate thread*/
				pthread_mutex_unlock(&arrayLock);
				return NULL;
			}
			/*Release lock before waiting*/
			pthread_cond_wait(&queueEmpty, &arrayLock);
			/*Reacquire lock to continue*/
		}
		/*pop and signal requesters that the array is no longer full*/
		site = (char *) pop(&queue);
		pthread_cond_signal(&queueFull);
		/*release lock before executing dnslookup*/
		pthread_mutex_unlock(&arrayLock);
		if (dnslookup(site, ip, sizeof(ip)) == -1) {
			fprintf(stderr, "Bogus Hostname: %s\n", site);
			strncpy(ip, "", sizeof(ip));
		}
		/*Acquire lock before accessing shared output text file*/
		pthread_mutex_lock(&resultsLock);
		output = fopen(resolverLog, "ab");
		fprintf(output, "%s,%s\n", site, ip);
		fclose(output);
		pthread_mutex_unlock(&resultsLock);
		free(site);
	}
}

int main(int argc, char *argv[] ) {
	/*Return an error if not enough command line arguments are included*/
	if (argc < 6) {
		fprintf(stderr, "Not enough arguments");
		return -1;
	}
	struct timeval firstTime, finalTime;
	/*Start tracking runtime*/
	gettimeofday(&firstTime, NULL);
	/*Output files and performance.txt*/
	FILE *output;
	//FILE *perf;
	/*Assign local variables to command line arguments*/
	int numberRequesters = atoi(argv[1]);
	int numberResolvers = atoi(argv[2]);
	requesterLog = argv[3];
	resolverLog = argv[4];
	
	pthread_t requesterThreads[numberRequesters], resolverThreads[numberResolvers];
	/*Array of struct files to hold all input files, dynamically allocated so it will be shared by requester threads*/
	files *f = malloc(sizeof(files) * MAX_INPUT_FILES);
	/*Write first two lines of performance.txt*/
	/*perf = fopen("performance.txt", "w");
	fprintf(perf, "Number of requester threads is %d\n", numberRequesters);
	fprintf(perf, "Number of resolver threads is %d\n", numberResolvers);
	fclose(perf);*/
	/*Fill struct files with input files*/
	for (int i = 0; i < argc - 5; i++) {
		f[i].fileName = argv[i + 5];
		f[i].length = argc - 5;
		f[i].status = 1;
	}
	/*Initialize queue and condition variables*/
	queueInit(&queue);
	pthread_cond_init(&queueFull, NULL);
	pthread_cond_init(&queueEmpty, NULL);
	/*Need to rewrite output files before appending, also need to check for bogus output files*/
	output = fopen(requesterLog, "w");
	if (!output) {
		fprintf(stderr, "Bogus output file path: %s", requesterLog);
		return -1;
	}
	remove(requesterLog);
	fclose(output);
	output = fopen(resolverLog, "w");
	if (!output) {
		fprintf(stderr, "Bogus output file path: %s", requesterLog);
		return -1;
	}
	remove(resolverLog);
	fclose(output);
	/*Create requester and resolver threads*/
	for (int i = 0; i < numberRequesters; i++) {
		pthread_create(&requesterThreads[i], NULL, requesterStartRoutine, (void *) f);
	}
	for (int i = 0; i < numberResolvers; i++) {
		pthread_create(&resolverThreads[i], NULL, resolverStartRoutine, NULL);
	}
	/*Wait for requester threads to finish*/
	for (int i = 0; i < numberRequesters; i++) {
		pthread_join(requesterThreads[i], NULL);
	}
	/*Let resolvers know requesters are all finished*/
	completed = 1;
	/*Wait for resolvers threads to finish*/
	for (int i = 0; i < numberResolvers; i++) {
		pthread_join(resolverThreads[i], NULL);
	}
	/*Free allocated memory*/
	free(f);
	queueCleanup(&queue);
	/*Write last line of performance.txt*/
	gettimeofday(&finalTime, NULL);
	printf("Total runtime: %ld seconds\n", finalTime.tv_sec - firstTime.tv_sec);
	/*perf = fopen("performance.txt", "ab");
	fprintf(perf, "./multi-lookup: total time is %ld seconds\n", finalTime.tv_sec - firstTime.tv_sec);
	fclose(perf);*/

	return 0;
}
