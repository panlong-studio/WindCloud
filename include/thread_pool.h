#ifndef _THREAD_POOL_H_
#define _THREAD_POOL_H_

#include <stdint.h>
#include "queue.h"
#include <pthread.h>
typedef struct thread_pool{
    int num;
    pthread_t* thread_id_arr;
    queue_t queue;
    pthread_mutex_t lock;
    pthread_cond_t cond;
    int exitFlag;
}thread_pool_t;

void init_thread_pool(thread_pool_t* pool,int num);

#endif