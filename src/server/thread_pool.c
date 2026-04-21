#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include "thread_pool.h"
#include "worker.h"
#include "error_check.h"
#include "log.h"

/*
 *函数作用：初始化线程池
 * 参数 pool：线程池结构体地址
 * 参数 num：要创建多少个工作线程
 */
void init_thread_pool(thread_pool_t* pool,int num){
    // exitFlag=0 表示线程池当前处于正常工作状态
    pool->exitFlag=0;
    
    // 工作线程数量
    pool->num=num;
    
    // 初始化互斥锁，用来保护任务队列和 busy_fds
    pthread_mutex_init(&pool->lock,NULL);
    
    // 初始化条件变量，任务队列为空时工作线程会等待这个条件变量
    pthread_cond_init(&pool->cond,NULL);
    
    // 初始化任务队列
    memset(&pool->queue,0,sizeof(queue_t));
    
    // 初始化线程ID数组
    pool->thread_id_arr=(pthread_t*)malloc(num*sizeof(pthread_t));
    
    // 初始化工作线程参数数组
    // 为每个线程准备一个 worker_arg_t
    // 这样每个线程都能知道“自己是第几个线程”
    pool->worker_arg_arr=(worker_arg_t*)malloc(num*sizeof(worker_arg_t));

    // 初始化 busy_fds 数组
    // 用来记录每个线程当前正在处理哪个 client_fd，-1 代表没有处理任何 fd
    pool->busy_fds=(int*)malloc(num*sizeof(int));

    // 只要有一个 malloc 失败，就直接退出
    if(pool->thread_id_arr==NULL || pool->worker_arg_arr==NULL || pool->busy_fds==NULL){
        LOG_ERROR("线程池内存分配失败");
        exit(1);
    }

    // 初始状态下，没有线程正在处理客户端，所以全部设成 -1
    for(int idx=0;idx<num;++idx){
        pool->busy_fds[idx]=-1;
    }

    // 开始创建工作线程
    for(int idx=0;idx<num;++idx){
        // 告诉当前线程，线程池的地址
        pool->worker_arg_arr[idx].pool=pool;
        // 告诉当前线程，自己的编号是多少
        pool->worker_arg_arr[idx].index=idx;

        // pthread_create 成功返回 0，失败返回正整数错误码
        int ret=pthread_create(&pool->thread_id_arr[idx],
                               NULL,
                               thread_func,
                               (void*)&pool->worker_arg_arr[idx]);
        THREAD_ERROR_CHECK(ret,"创建工作线程");                                                  
    }
    LOG_INFO("线程池初始化完成，工作线程数=%d", num);
}

/* 
 *函数作用：销毁线程池申请过的资源。
 * 参数 pool：线程池结构体地址。
 */
void destroy_thread_pool(thread_pool_t *pool){
    // 空指针保护
    if(pool==NULL){
        return;
    }

    // 释放资源（与malloc一一对应）
    free(pool->thread_id_arr);
    free(pool->worker_arg_arr);
    free(pool->busy_fds);

    // 销毁同步工具
    pthread_mutex_destroy(&pool->lock);
    pthread_cond_destroy(&pool->cond);
}