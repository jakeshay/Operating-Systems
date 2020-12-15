#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#define ARRAY_SIZE 20

typedef struct BufferEntry {
	void* line;
} entry;

typedef struct Buffer {
	entry* array;
	int head;
	int tail;
} buffer;

void queueInit(buffer *buffer);
bool isEmpty(buffer *buffer);
bool isFull(buffer *buffer);
void push(void* name, buffer *buffer);
void *pop(buffer *buffer);
void queueCleanup(buffer *buffer);
