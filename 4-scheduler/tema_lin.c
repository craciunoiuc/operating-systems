#include "./tema_lin.h"

int so_init(unsigned int time_quantum, unsigned int io)
{
	int ret;

	if (time_quantum == 0 || io > SO_MAX_NUM_EVENTS || planner != NULL)
		return -1;

	io_devices = io;
	time_to_check = time_quantum;
	planner = pqueue_new(compare_threads, SO_MAX_THREADS);
	ret = pthread_mutex_init(&mutex_core, NULL);
	DIE(ret != 0, "Mutex Init");
	return 0;
}

tid_t so_fork(so_handler *func, unsigned int priority)
{
	int ret;
	tdata_t *thread;

	if (func == 0 || priority > SO_MAX_PRIO)
		return INVALID_TID;

	thread = malloc(sizeof(tdata_t));
	if (thread == NULL)
		exit(12);

	thread->func = func;
	thread->priority = priority;
	thread->remaining_instr = time_to_check;

	ret = pthread_create(&thread->thread, NULL, &start_thread, thread);
	DIE(ret != 0, "Thread Create");
	pqueue_enqueue(planner, thread);
	return thread->thread;
}

void *start_thread(void *params)
{
	int return_val;
	tdata_t *data = (tdata_t *)params;

	return_val = pthread_mutex_lock(&mutex_core);
	DIE(return_val != 0, "Mutex Lock");
	(*data->func)(data->priority);
	return_val = pthread_mutex_unlock(&mutex_core);
	DIE(return_val != 0, "Mutex Unlock");
	return NULL;
}

int so_wait(unsigned int io)
{
	if (io >= io_devices)
		return -1;
	return 0;
}

int so_signal(unsigned int io)
{
	if (io >= io_devices)
		return -1;
	return 0;
}

void so_exec(void)
{
}

void so_end(void)
{
	int ret;
	tdata_t *td;

	if (planner != NULL) {
		while (planner->size != 0) {
			td = pqueue_dequeue(planner);
			if (pthread_join(td->thread, NULL))
				perror("pthread_join");
			free(td);
		}
		pqueue_delete(planner);
		ret = pthread_mutex_destroy(&mutex_core);
		DIE(ret != 0, "Mutex Destroy");
		planner = NULL;
	}
}
