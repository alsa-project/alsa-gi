#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <pthread.h>
#include <sys/epoll.h>
#include <sys/queue.h>

#include "context.h"

#define local_malloc(ptr, length, errptr) \
        (ptr) = malloc((length)); \
        if (!(errptr)) \
                (*errptr) = ENOMEM; \
        else \
                memset((ptr), 0, (length));

#define local_free(ptr) \
        if (ptr) \
                free(ptr); \
        ptr = NULL;

#define INITIAL_EVENT_COUNT	10
#define INITIAL_EVENT_TIMEOUT	1000

struct alsa_context_reactant {
	unsigned int reactor_id;
	int fd;
	uint32_t events;
	ALSAContextReactantCallback callback;
	void *private_data;
};

struct alsa_context_reactor {
	unsigned int id;
	bool active;

	int epfd;
	unsigned int result_count;
	struct epoll_event *results;
	unsigned int period_msec;

	pthread_t thread;
	pthread_mutex_t lock;
	LIST_ENTRY(alsa_context_reactor) link;
};

static LIST_HEAD(reactor_list_head, alsa_context_reactor) reactors =
			LIST_HEAD_INITIALIZER(alsa_context_reactor_head);
static pthread_mutex_t reactors_lock = PTHREAD_MUTEX_INITIALIZER;

static void reactant_prepared(struct alsa_context_reactant *reactant,
			      uint32_t event, int *err)
{
	ALSAContextReactantState state;

	if (!reactant->callback)
		return;

	if (event & EPOLLIN)
		state = ALSAContextReactantStateReadable;
	else if (event & EPOLLOUT)
		state = ALSAContextReactantStateWritable;
	else if (event & (EPOLLERR | EPOLLHUP))
		state = ALSAContextReactantStateError;
	else
		return;

	reactant->callback(state, reactant->private_data, err);
}

static void clean_reactor(void *arg)
{
	struct alsa_context_reactor *reactor = arg;

	reactor->thread = 0;
	close(reactor->epfd);
	local_free(reactor->results);
}

static void running_reactor(struct alsa_context_reactor *reactor)
{
	struct alsa_context_reactant *reactant;
	struct epoll_event *ev;
	uint32_t event;
	unsigned int i;
	int count, err;

	pthread_cleanup_push(clean_reactor, (void *)reactor);

	reactor->active = true;

	while (reactor->active) {
		/*
		 * epoll_wait(7) is designed so as thread-safe. Here, let's
		 * block this thread till any file descriptors are added and
		 * any events occur.
		 */
		count = epoll_wait(reactor->epfd, reactor->results,
				   reactor->result_count, -1);
		if (count < 0) {
			printf("%s\n", strerror(errno));
			reactor->active = false;
			continue;
		}

		for (i = 0; i < count; i++) {
			ev = &reactor->results[i];
			event = ev->events;
			reactant = (struct alsa_context_reactant *)ev->data.ptr;
			reactant_prepared(reactant, event, &err);
		}
	}

	pthread_exit(NULL);
	pthread_cleanup_pop(true);
}

ALSAContextReactor *alsa_context_reactor_create(unsigned int id, int *err)
{
	struct alsa_context_reactor *reactor;

	/* Check a reactor with the same id is already in a list or not */
	pthread_mutex_lock(&reactors_lock);
	LIST_FOREACH(reactor, &reactors, link) {
		if (reactor->id == id)
			return NULL;
	}
	pthread_mutex_unlock(&reactors_lock);

	/* Keep an instance of reactor. */
	local_malloc(reactor, sizeof(struct alsa_context_reactor), err);
	if (*err)
		return NULL;
	reactor->id = id;

	/* Keep a buffer for epoll_wait(7) result. */
	reactor->result_count = INITIAL_EVENT_COUNT;
	local_malloc(reactor->results,
		     sizeof(struct epoll_event) * reactor->result_count, err);
	if (*err) {
		local_free(reactor);
		return NULL;
	}
	reactor->period_msec = INITIAL_EVENT_TIMEOUT;
	pthread_mutex_init(&reactor->lock, NULL);

	/* Insert this instance into the list. */
	pthread_mutex_lock(&reactors_lock);
	LIST_INSERT_HEAD(&reactors, reactor, link);
	pthread_mutex_unlock(&reactors_lock);

	return reactor;
}

void alsa_context_reactor_start(ALSAContextReactor *reactor, int *err)
{
	pthread_attr_t attr;

	/* Prepare an instance of epoll(7) . */
	reactor->epfd = epoll_create1(EPOLL_CLOEXEC);
	if (reactor->epfd < 0) {
		*err = errno;
		return;
	}

	/* Prepare and start an thread. */
	*err = -pthread_attr_init(&attr);
	if (*err)
		goto end;

	*err = -pthread_attr_setinheritsched(&attr, PTHREAD_INHERIT_SCHED);
	if (*err)
		goto end;

	*err = -pthread_create(&reactor->thread, &attr,
			      (void *)running_reactor, (void *)reactor);
end:
	if (*err)
		close(reactor->epfd);
}

void alsa_context_reactor_stop(ALSAContextReactor *reactor)
{
	int err;

	reactor->active = false;

	if (reactor->thread)
		pthread_join(reactor->thread, NULL);
}

void alsa_context_reactor_destroy(ALSAContextReactor *reactor)
{
	pthread_mutex_lock(&reactors_lock);
	LIST_REMOVE(reactor, link);
	pthread_mutex_unlock(&reactors_lock);

	local_free(reactor);
}

ALSAContextReactant *alsa_context_reactant_create(unsigned int reactor_id,
					int fd,
					ALSAContextReactantCallback callback,
					void *private_data, int *err)
{
	ALSAContextReactant *reactant;

	local_malloc(reactant, sizeof(ALSAContextReactant), err);
	if (*err)
		return NULL;

	reactant->reactor_id = reactor_id;
	reactant->fd = fd;
	reactant->callback = callback;
	reactant->private_data = private_data;

	return reactant;
}

void alsa_context_reactant_add(ALSAContextReactant *reactant, uint32_t events,
			       int *err)
{
	struct alsa_context_reactor *reactor;
	struct epoll_event ev;

	reactant->events = events;

	pthread_mutex_lock(&reactors_lock);

	LIST_FOREACH(reactor, &reactors, link) {
		if (reactor->id == reactant->reactor_id)
			break;
	}

	if (reactor == NULL) {
		*err = -EINVAL;
		goto end;
	}

	ev.events = reactant->events;
	ev.data.ptr = (void *)reactant;

	if (epoll_ctl(reactor->epfd, EPOLL_CTL_ADD, reactant->fd, &ev) < 0)
		*err = errno;
end:
	pthread_mutex_unlock(&reactors_lock);
}

void alsa_context_reactant_remove(ALSAContextReactant *reactant)
{
	struct alsa_context_reactor *reactor;
	int err;

	pthread_mutex_lock(&reactors_lock);
	LIST_FOREACH(reactor, &reactors, link)
		epoll_ctl(reactor->epfd, EPOLL_CTL_DEL, reactant->fd, NULL);
	pthread_mutex_unlock(&reactors_lock);

	reactant->fd = 0;
}

void alsa_context_reactant_destroy(ALSAContextReactant *reactant)
{
	alsa_context_reactant_remove(reactant);
	local_free(reactant);
}
