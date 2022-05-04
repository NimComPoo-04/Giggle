#include <stdio.h>
#include <stdlib.h>

#include <unistd.h>

#include "tpool.h"

/*
 * TODO: the error handling here is very poor the thread creation destruction and
 * all the other standard pthread errors are not check.
 */

static void *thread_handler(void *);
static struct task *tpool_task_query(tpool_t *t);

tpool_t *tpool_create(int tnum)
{
	tpool_t *t = malloc(sizeof(tpool_t));

	t->threads = calloc(sizeof(struct task), tnum);
	t->tnum = tnum;
	t->task_current = NULL;
	t->task_tail = NULL;

	pthread_mutex_init(&t->mutex, NULL);

	for(int i = 0; i < tnum; i++)
	{
		t->threads[i].status = THREAD_WAITING;
		t->threads[i].is_dead = 0;
		t->threads[i].tsk = NULL;
		pthread_create(&t->threads[i].tid, NULL, thread_handler, t);
	}
	return t;
}

void tpool_exec(tpool_t *t, void(*handle)(connection_t *), connection_t arg)
{
	// using the same mutex to block the read operations.
	// since the read operation might modify the queue
	pthread_mutex_lock(&t->mutex);

	struct task *tsk = malloc(sizeof(struct task));
	tsk->handle = handle;
	tsk->arg = arg;
	tsk->next = NULL;

	// if the task_current is empty then there are no jobs to be done
	if(t->task_current == NULL)
	{
		t->task_current = tsk;
		t->task_tail = t->task_current;
		pthread_mutex_unlock(&t->mutex);
		return;
	}

	t->task_tail->next = tsk;
	t->task_tail = tsk;

	pthread_mutex_unlock(&t->mutex);
}

void tpool_destroy(tpool_t *t)
{
	for(int i = 0; i < t->tnum; i++)
	{
		// this should indicate to the thread that it should return
		t->threads[i].is_dead = 1;
	}

	for(int i = 0; i < t->tnum; i++)
		pthread_join(t->threads[i].tid, NULL);

	free(t->threads);

	while(t->task_current)
	{
		struct task *k = t->task_current->next;
		free(t->task_current);
		t->task_current = k;
	}

	pthread_mutex_destroy(&t->mutex);

	free(t);
}

// INTERNAL FUNCTION

static void *thread_handler(void *g)
{
	tpool_t *tpool= g; // casting the pointer for memory safety

	// finding out whoami
	pthread_t self_tid = pthread_self(); 
	struct thread *self = NULL;

	// if the self == NULL this means the thread has not been registered yet
	// we loop until we reach the correct condition lol
	while(self == NULL)
	{
		for(int i = 0; i < tpool->tnum; i++)
		{
			if(self_tid == tpool->threads[i].tid)
				self = &tpool->threads[i];
		}
	}
	
	while(1)
	{
		// thread returns if it encouters is_dead
		if(self->is_dead) goto EXT;
		if(self->status == THREAD_WAITING)
		{
			// asks for task from the tpool
			struct task *t = tpool_task_query(tpool);

			// if t is null then there are no tasks left to be executed
			// in that case we don't care otherwise we do
			if(t != NULL)
			{
				self->tsk = t;
				self->status = THREAD_RUNNING;
			}
		}
		if(self->status == THREAD_RUNNING)
		{
			self->tsk->handle(&self->tsk->arg);
			self->status = THREAD_WAITING;

			// remove the completed task from queue
			free(self->tsk);
		}
	}
EXT:

	return NULL;
}

/*
 * NOTE: the following functions need to be thread safe
 * otherwise 2 threads might try to do the same task
 *
 * This function has the potential to slow everything down 
 * dramatically have to come up with a better queue tech honestly
 */

static struct task *tpool_task_query(tpool_t *t)
{
	pthread_mutex_lock(&t->mutex);

	struct task *tsk = t->task_current;

	if(t->task_current != NULL)
		t->task_current = t->task_current->next;

	pthread_mutex_unlock(&t->mutex);

	return tsk;
}
