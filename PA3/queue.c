#include "queue.h"

void queueInit(buffer *buffer) {
	buffer -> array = malloc(sizeof(entry) * ARRAY_SIZE);
	buffer -> head = -1;
	buffer -> tail = 0;
	
	for (int i = 0; i < ARRAY_SIZE; i++) {
		buffer -> array[i].line = NULL;
	}
}

bool isEmpty(buffer *buffer) {
	return buffer -> head == -1;
}

bool isFull(buffer *buffer) {
	return buffer -> head == ARRAY_SIZE - 1;
}

void push(void *name, buffer *buffer) {
	if (!isFull(buffer)) {
		buffer -> array[buffer -> head + 1].line = name;
		buffer -> head += 1;
	}
}

void *pop(buffer *buffer) {
	void *temp;
	if (!isEmpty(buffer)) {
		temp = buffer -> array[buffer -> head].line;
		buffer -> head -= 1;
		return temp;
	}
	return NULL;
}

void queueCleanup(buffer *buffer) {
	while (!isEmpty(buffer)) {
		pop(buffer);
	}
	free(buffer -> array);
}
