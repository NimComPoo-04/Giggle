#ifndef _TPOOL_H_
#define _TPOOL_H_

#include <pthread.h>
#include "connection.h"

enum
{
	THREAD_RUNNING,
	THREAD_WAITING
};

struct task
{
	void(*handle)(connection_t *);
	connection_t arg;
	struct task *next;
};

struct thread
{
	pthread_t tid;	// NOTE: tid does not indicate the hardware thread id
	int status;
	int is_dead;	// to kill the thread if needed
	struct task *tsk; // TODO: find better name for this dude
};

typedef struct
{
	struct thread *threads;
	int tnum;
	struct task *task_current;
	struct task *task_tail;
	pthread_mutex_t mutex;
} tpool_t;

tpool_t *tpool_create(int tnum);
void tpool_exec(tpool_t *t, void(*handle)(connection_t *), connection_t arg);
void tpool_destroy(tpool_t *t);

#endif
