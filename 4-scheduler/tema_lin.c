#include "./tema_lin.h"

int so_init(unsigned int time_quantum, unsigned int io)
{
    if (time_quantum == 0 || io > SO_MAX_NUM_EVENTS || planner != NULL)
        return -1;
    io_devices = io;
    time_to_check = time_quantum;
    planner = pqueue_new(compare_threads, SO_MAX_THREADS);
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
    if (planner != NULL) {
        pqueue_delete(planner);
        planner = NULL;
    }
}
