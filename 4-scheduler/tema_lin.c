#include <stdio.h>
#include "./so_scheduler.h"

int so_init(unsigned int time_quantum, unsigned int io)
{
    return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
    tid_t thread;
    return thread;
}

int so_wait(unsigned int io)
{
    return 0;
}

int so_signal(unsigned int io)
{
    return 0;
}

void so_exec()
{

}

void so_end()
{

}
