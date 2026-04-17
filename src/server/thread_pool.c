#include <my_header.h>
#include "thread_pool.h"
#include "worker.h"


void init_thread_pool(thread_pool_t* pool,int num){
    pool->exitFlag=0;
    pool->num=num;
    pthread_mutex_init(&pool->lock,NULL);
    pthread_cond_init(&pool->cond,NULL);
    memset(&pool->queue,0,sizeof(queue_t));
    pool->thread_id_arr=(pthread_t*)malloc(num*sizeof(pthread_t));

    for(int idx=0;idx<num;++idx){
        int ret=pthread_create(pool->thread_id_arr[idx],NULL,&thread_func,(void*) pool);  //pthread_create的入参：得到的pthread_id存入
        ERROR_CHECK(ret,-1,"pthread_create");                                                    //②NULL ③thread_func  ④thread_func的入参
    }

}