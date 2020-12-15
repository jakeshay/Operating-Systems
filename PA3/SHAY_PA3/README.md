multi-lookup:
	.h: header file for multi-lookup. Includes "util.h" and "queue.h". Defines global limits and thread start routines for requesters and resolvers.
	.c: implemenation of thread start routines, and driver code which creates the thread pools.

queue:
	.h: header file for shared array implementation. Includes function definitions such as push, pop, init, etc.
	.c: shared array implementation
