#include "multi-lookup.h"

pthread_mutex_t fileLock, servicedLock, arrayLock, resultsLock;
pthread_cond_t queueFull, queueEmpty;

void *requesterStartRoutine(void *arg) {
	ts *t = (ts *) arg;
	/*Input files, output file, and performance.txt*/
	FILE *fp, *output;
	//FILE *perf;
	/*Count files serviced by thread*/
	int filesServiced = 0;
	/*Hold current line ready by current input files*/
	char line[MAX_NAME_LENGTH];
	/*Loop through input files*/
	for (int i = 0; i < t -> numFiles; i++) {
		/*Lock access while updating status of current file*/
		pthread_mutex_lock(&fileLock);
		if (t -> f[i].status) {
			t -> f[i].status = 0;
			/*Update status to accessed*/
			pthread_mutex_unlock(&fileLock);
			fp = fopen(t -> f[i].fileName, "r");
			/*Return error if bogus input file*/
			if (!fp) {
				fprintf(stderr, "Bogus input file path: %s\n", t -> f[i].fileName);
			}
			else {
				/*Read file line by line*/
				while(fgets(line, MAX_NAME_LENGTH, fp)) {
					/*Remove new line character so DNS will work*/
					strtok(line, "\n");
					/*Lock access while pushing to shared array*/
					pthread_mutex_lock(&arrayLock);
					while (isFull(&t -> queue)) {
						/*Wait til array is no longer full*/
						pthread_cond_wait(&queueFull, &arrayLock);
					}
					/*Push to array*/
					push(strdup(line), &t -> queue);
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
	output = fopen(t -> requesterLog, "ab");
	//perf = fopen("performance.txt", "ab");
	fprintf(output, "Thread %lu serviced %d files.\n", pthread_self(), filesServiced);
	//fprintf(perf, "Thread %lu serviced %d files.\n", pthread_self(), filesServiced);
	fclose(output);
	//fclose(perf);
	pthread_mutex_unlock(&servicedLock);
	return NULL;
}

void *resolverStartRoutine(void *arg) {
	ts *t = (ts *) arg;
	char *site;
	FILE *output;
	char ip[MAX_IP_LENGTH];
	while (1) {
		/*Lock while accessing shared array*/
		pthread_mutex_lock(&arrayLock);
		while (isEmpty(&t -> queue)) {
			/*If array is empty and requesters are finished begin exit*/
			if (t -> completed) {
				/*release the lock and terminate thread*/
				pthread_mutex_unlock(&arrayLock);
				return NULL;
			}
			/*Release lock before waiting*/
			pthread_cond_wait(&queueEmpty, &arrayLock);
			/*Reacquire lock to continue*/
		}
		/*pop and signal requesters that the array is no longer full*/
		site = (char *) pop(&t -> queue);
		pthread_cond_signal(&queueFull);
		/*release lock before executing dnslookup*/
		pthread_mutex_unlock(&arrayLock);
		if (dnslookup(site, ip, sizeof(ip)) == -1) {
			fprintf(stderr, "Bogus Hostname: %s\n", site);
			strncpy(ip, "", sizeof(ip));
		}
		/*Acquire lock before accessing shared output text file*/
		pthread_mutex_lock(&resultsLock);
		output = fopen(t -> resolverLog, "ab");
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

	pthread_t requesterThreads[numberRequesters], resolverThreads[numberResolvers];
	/*Array of struct files to hold all input files, dynamically allocated so it will be shared by requester threads*/
	ts *threadArg = malloc(sizeof(ts));
	threadArg -> f = malloc(sizeof(files) * MAX_INPUT_FILES);
	threadArg -> requesterLog = argv[3];
	threadArg -> resolverLog = argv[4];
	/*Write first two lines of performance.txt*/
	/*perf = fopen("performance.txt", "w");
	fprintf(perf, "Number of requester threads is %d\n", numberRequesters);
	fprintf(perf, "Number of resolver threads is %d\n", numberResolvers);
	fclose(perf);*/
	/*Fill struct files with input files*/
	for (int i = 0; i < argc - 5; i++) {
		threadArg -> f[i].fileName = argv[i + 5];
		threadArg -> f[i].status = 1;
	}
	threadArg -> numFiles = argc - 5;
	/*Initialize queue and condition variables*/
	queueInit(&threadArg -> queue);
	pthread_cond_init(&queueFull, NULL);
	pthread_cond_init(&queueEmpty, NULL);
	/*Need to rewrite output files before appending, also need to check for bogus output files*/
	output = fopen(threadArg -> requesterLog, "w");
	if (!output) {
		fprintf(stderr, "Bogus output file path: %s", threadArg -> requesterLog);
		return -1;
	}
	remove(threadArg -> requesterLog);
	fclose(output);
	output = fopen(threadArg -> resolverLog, "w");
	if (!output) {
		fprintf(stderr, "Bogus output file path: %s", threadArg -> requesterLog);
		return -1;
	}
	remove(threadArg -> resolverLog);
	fclose(output);
	/*Create requester and resolver threads*/
	for (int i = 0; i < numberRequesters; i++) {
		pthread_create(&requesterThreads[i], NULL, requesterStartRoutine, (void *) threadArg);
	}
	for (int i = 0; i < numberResolvers; i++) {
		pthread_create(&resolverThreads[i], NULL, resolverStartRoutine, (void *) threadArg);
	}
	/*Wait for requester threads to finish*/
	for (int i = 0; i < numberRequesters; i++) {
		pthread_join(requesterThreads[i], NULL);
	}
	/*Let resolvers know requesters are all finished*/
	threadArg -> completed = 1;
	/*Wait for resolvers threads to finish*/
	for (int i = 0; i < numberResolvers; i++) {
		pthread_join(resolverThreads[i], NULL);
	}
	/*Free allocated memory*/
	free(threadArg -> f);
	free(threadArg);
	queueCleanup(&threadArg -> queue);
	/*Write last line of performance.txt*/
	gettimeofday(&finalTime, NULL);
	printf("Total runtime: %ld seconds\n", finalTime.tv_sec - firstTime.tv_sec);
	/*perf = fopen("performance.txt", "ab");
	fprintf(perf, "./multi-lookup: total time is %ld seconds\n", finalTime.tv_sec - firstTime.tv_sec);
	fclose(perf);*/

	return 0;
}
