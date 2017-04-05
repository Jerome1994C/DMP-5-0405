/***************************************************************************
    begin                : Tue Oct 14 2003
    copyright            : (C) 2003 - 2009 by Alper Akcan
    email                : alper.akcan@gmail.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU Lesser General Public License as        *
 *   published by the Free Software Foundation; either version 2.1 of the  *
 *   License, or (at your option) any later version.                       *
 *                                                                         *
 ***************************************************************************/

#ifndef THREAD_H
#define THREAD_H

#include <stdint.h>

#define CONFIG_DEBUG

#if defined(__cplusplus)
	extern "C" {
#endif

#if defined(CONFIG_THREAD_WINDOWS)
#include <stdlib.h>
#include <stdio.h>
struct s_thread_sem_s {
	void* sem_id;
};
#endif

#if defined(CONFIG_THREAD_PTHREADS)
#include <semaphore.h>
struct s_thread_sem_s {
	sem_t *sem;
	sem_t sem_data;
};
#endif

#define DFAT	1
#define DSYS	2
#define DSER	4
#define DCLI	8

typedef struct s_thread_s s_thread_t;
typedef struct s_thread_sem_s s_thread_sem_t;
typedef struct s_thread_cond_s s_thread_cond_t;
typedef struct s_thread_mutex_s s_thread_mutex_t;

typedef void * (*s_thread_func_t)(void *);

struct s_thread_mutex_s {
	int recursive;
	int owner;
	s_thread_sem_t *sem;
};

struct s_thread_cond_s {
	s_thread_mutex_t *lock;
	int waiting;
	int signals;
	s_thread_sem_t *wait_sem;
	s_thread_sem_t *wait_done;
};

typedef struct s_thread_arg_s {
	int flag;
	void *arg;
	void * (*r) (void *);
	void * (*f) (void *);
	s_thread_cond_t *cond;
	s_thread_mutex_t *mut;
} s_thread_arg_t;

typedef struct s_thread_api_s {
	int (*sem_create) (s_thread_sem_t *sem, int initial);
	int (*sem_destroy) (s_thread_sem_t *sem);
	int (*sem_wait) (s_thread_sem_t *sem);
	int (*sem_wait_timeout) (s_thread_sem_t *sem, int msec);
	int (*sem_post) (s_thread_sem_t *sem);

	int (*mutex_init) (s_thread_mutex_t *mut);
	int (*mutex_destroy) (s_thread_mutex_t *mut);
	int (*mutex_lock) (s_thread_mutex_t *mut);
	int (*mutex_trylock) (s_thread_mutex_t *mut);
	int (*mutex_unlock) (s_thread_mutex_t *mut);

	int (*cond_init) (s_thread_cond_t *cond);
	int (*cond_destroy) (s_thread_cond_t *cond);
	int (*cond_signal) (s_thread_cond_t *cond);
	int (*cond_broadcast) (s_thread_cond_t *cond);
	int (*cond_wait) (s_thread_cond_t *cond, s_thread_mutex_t *mut);
	int (*cond_timedwait) (s_thread_cond_t *cond, s_thread_mutex_t *mut, int msec);

	int (*thread_create) (s_thread_t *tid, s_thread_arg_t *targ);
	int (*thread_cancel) (s_thread_t *tid);
	int (*thread_join) (s_thread_t *tid, void **ret);
	int (*thread_detach) (s_thread_t *tid);
	int (*thread_self) (void);
	void (*thread_exit) (void *ret);
} s_thread_api_t;

/* alloc.c */
void * s_malloc (unsigned int size);
void * s_calloc (unsigned int nmemb, unsigned int size);
void * s_realloc (void *ptr, unsigned int size);
void s_free (void *ptr);
void s_trace_start (void);
void s_trace_stop (void);

/*@}*/

/* debugf.c */
void s_debug_debugf (unsigned short flags, char *file, int line, char *func, char *fmt, ...);


/** @defgroup client_time Client Library - Time API
  * @brief this is a layer to operating systems time api.
  */

/** @addtogroup client_time */
/*@{*/

/* gettime.c */

/** @brief resolution is miliseconds, and is enough for any gui related work.
  *        if you need a high presicion you should not use this function,
  *        consider looking to your system specs.
  *
  * @returns the time value in miliseconds
  */
long long s_gettimeofday (void);

/*@}*/

/** @defgroup client_grlib Client Library - Graphics API
  * @brief these are simple low-level graphics api functions, for further information
  *        you may look in demo, and the server source codes.
  *
  * @example
  * @code
  * // simple example code will be in here
  * @endcode
  */

/** @addtogroup client_grlib */
/*@{*/

/* thread.c */

/** @brief initialize the semaphore struct with an initial value
  *
  * @param **sem   - address of the semaphore pointer.
  * @param initial - initial value.
  * @returns 0 on success, 1 on error.
  */
int s_thread_sem_create (s_thread_sem_t **sem, int initial);

/** @brief destroys the given semaphore
  *
  * @param *sem - address of the semaphore pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_sem_destroy (s_thread_sem_t *sem);

/** @brief waits on given semaphore
  *
  * @param *sem - address of the semaphore pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_sem_wait (s_thread_sem_t *sem);

/** @brief waits on given semaphore with timeout
  *
  * @param *sem - address of the semaphore pointer.
  * @param msec - timeout in mili seconds.
  * @returns 0 on success, 1 on timeout, -1 on error.
  */
int s_thread_sem_wait_timeout (s_thread_sem_t *sem, int msec);

/** @brief unlocks given semaphore
  *
  * @param *sem - address of the semaphore pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_sem_post (s_thread_sem_t *sem);

/** @brief initialize the mutex struct
  *
  * @param **mut - address of the mutex pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_mutex_init (s_thread_mutex_t **mut);

/** @brief destroys the given mutex
  *
  * @param *mut - address of the mutex pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_mutex_destroy (s_thread_mutex_t *mut);

/** @brief locks the given mutex
  *
  * @param *mut - address of the mutex pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_mutex_lock (s_thread_mutex_t *mut);

/** @brief tries to lock the given mutex
  *
  * @param *mut - address of the mutex pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_mutex_trylock (s_thread_mutex_t *mut);

/** @brief unlocks the given mutex
  *
  * @param *mut - address of the mutex pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_mutex_unlock (s_thread_mutex_t *mut);

/** @brief initialize the condition variable struct
  *
  * @param **cond - address of the condition variable pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cond_init (s_thread_cond_t **cond);

/** @brief destroys the given condition variable
  *
  * @param *cond - address of the condition variable pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cond_destroy (s_thread_cond_t *cond);

/** @brief signals waiter on given condition variable
  *
  * @param *cond - address of the condition variable pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cond_signal (s_thread_cond_t *cond);

/** @brief signals all waiters on given condition variable
  *
  * @param *cond - address of the condition variable pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cond_broadcast (s_thread_cond_t *cond);

/** @brief waits on condition variable
  *
  * @param *cond - address of the condition variable pointer.
  * @param *mut  - address of the mutex pointer.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cond_wait (s_thread_cond_t *cond, s_thread_mutex_t *mut);

/** @brief waits on condition variable, with timeout
  *
  * @param *cond - address of the condition variable pointer.
  * @param *mut  - address of the mutex pointer.
  * @param *msec - timeout value in mili seconds.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cond_timedwait (s_thread_cond_t *cond, s_thread_mutex_t *mut, int msec);

/** @brief creates a new thread of control that executes concurrently
  *        with the calling thread. The new thread applies the function
  *        passing it as first argument.
  *
  * @param *f   - pointer to the function.
  * @param *farg - argument to pass to function.
  * @returns NULL on error, otherwise address of the thread id.
  */
s_thread_t * s_thread_create (void * (*f) (void *), void *farg);

/** @brief cancels the thread.
  *
  * @param *tid - thread id of the thread to cancel.
  * @returns 0 on success, 1 on error.
  */
int s_thread_cancel (s_thread_t *tid);

/** @brief suspends the execution of the calling thread until the
  *        thread identified by tid terminates, either by calling
  *        s_thread_exit or by being cancelled.
  *
  * @param *tid  - thread id of the thread to cancel.
  * @param **ret - return value of the thread.
  * @returns 0 on success, 1 on error.
  */
int s_thread_join (s_thread_t *tid, void **ret);

/** @brief detachs the given thread
  *
  * @param *tid  - thread id of the thread to detach.
  * @returns 0 on success, 1 on error.
  */
int s_thread_detach (s_thread_t *tid);

/** @brief returns the thread identifier for the calling thread.
  *
  * @returns th thread id.
  */
int s_thread_self (void);

/** @brief terminates the execution of the calling thread.
  */
void s_thread_exit (void *ret);

enum {
    DBG_SYS  = 1<<0,
    DBG_DEVICE = 1<<4,
};

#ifdef __cplusplus
	}
#endif

#if defined(CONFIG_DEBUG)
	#define debugf(a, b...)	s_debug_debugf(a, __FILE__, __LINE__, (char *) __FUNCTION__, b);
#else
	#define debugf(a, b...) {\
		if ((a) & DFAT) {\
			s_debug_debugf(a, __FILE__, __LINE__, (char *) __FUNCTION__, b);\
		}\
	}
#endif

#include <time.h>

struct s_thread_timer
{
     time_t expire;
     time_t runtime;
};

int s_thread_timer_init(struct s_thread_timer *timer, uint32_t sec);
void s_thread_timer_start(struct s_thread_timer *timer);
int s_thread_timer_expired(struct s_thread_timer *timer);
void s_thread_timer_term(struct s_thread_timer *timer);
int s_thread_timer_elapsed(struct s_thread_timer *timer);

#endif /* THREAD_H */
