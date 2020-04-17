#ifndef _TEMA_LIN
#define _TEMA_LIN

#include <stdio.h>
#include <stdlib.h>
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
static unsigned int io_devices = 0;
static unsigned int time_to_check = 0;

static PQueue *planner = NULL;

void *start_thread(void *params);

int compare_threads(const void *thread1, const void *thread2)
{
	const tdata_t *elem1 = thread1;
	const tdata_t *elem2 = thread2;
	return elem2->priority - elem1->priority;
}

#endif /*_TEMA_LIN*/