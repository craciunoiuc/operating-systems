#ifndef _TEMA_LIN
#define _TEMA_LIN

#include <stdio.h>
#include <stdlib.h>
#include <semaphore.h>
#include "./so_scheduler.h"
#include "./priorityqueue.h"

#define SO_MAX_THREADS 8192

typedef struct {
	unsigned int priority;
	unsigned int remaining_instr;
	so_handler *func;
	tid_t thread;
} tdata_t;

static pthread_mutex_t mutex_core;
static unsigned int io_devices, time_to_check;
static PQueue *planner;

void *start_thread(void *params);

int compare_threads(const void *thread1, const void *thread2)
{
	const tdata_t *elem1 = thread1;
	const tdata_t *elem2 = thread2;

	return elem1->priority - elem2->priority;
}

#endif /*_TEMA_LIN*/
