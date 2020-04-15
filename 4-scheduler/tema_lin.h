#ifndef _TEMA_LIN
#define _TEMA_LIN
#include <stdio.h>
#include "./so_scheduler.h"
#include "./priorityqueue.h"

#define SO_MAX_THREADS 256

typedef struct {
    unsigned int priority;
    unsigned int tid;
    tid_t thread;
} tdata_t;

static unsigned int io_devices = 0;
static unsigned int time_to_check = 0;

static PQueue *planner = NULL;

int compare_threads(const tdata_t *thread1, const tdata_t *thread2)
{
    return thread1->priority - thread2->priority;
}

#endif /*_TEMA_LIN*/