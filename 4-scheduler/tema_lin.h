#ifndef _TEMA_LIN
#define _TEMA_LIN

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "./so_scheduler.h"
#include "./priorityqueue.h"

/* Hard upper limit for the queue */
#define SO_MAX_THREADS 8192

typedef struct {
	unsigned int priority;
	unsigned int remaining_instr;
	so_handler *func;
	tid_t thread;
} tdata_t;

/* Mutex to limit threads to "1 core" */
static pthread_mutex_t mutex_core;
/* Constants provided by the tests */
static unsigned int io_devices, time_to_check;
/* Priority queue */
static PQueue *planner;

/* Auxiliary function to run the threads */
void *start_thread(void *params);

/* Compare function for the queue */
int compare_threads(const void *thread1, const void *thread2)
{
	const tdata_t *elem1 = thread1;
	const tdata_t *elem2 = thread2;

	return elem1->priority - elem2->priority;
}

/* DIE macro from the laboratory to crash on system call fail */
#define DIE(assertion, call_description)				\
	do {								\
		if (assertion) {					\
			fprintf(stderr, "(%s, %d): ",			\
					__FILE__, __LINE__);		\
			perror(call_description);			\
			exit(EXIT_FAILURE);				\
		}							\
	} while (0)


#endif /*_TEMA_LIN*/
