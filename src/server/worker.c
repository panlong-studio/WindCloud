#include "worker.h"
#include "thread_pool.h"
#include "queue.h"
#include "handle.h"
#include <stdio.h>
#include <unistd.h>

void* thread_func(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;
    
    while (1) {
        pthread_mutex_lock(&pool->lock);
        
        // 等待任务
        while (queueIsEmpty(&pool->queue) && !pool->exitFlag) {
            pthread_cond_wait(&pool->cond, &pool->lock);
        }
        
        // 退出标志
        if (pool->exitFlag) {
            pthread_mutex_unlock(&pool->lock);
            break;
        }
        
        // 取出客户端fd
        int client_fd = deQueue(&pool->queue);
        pthread_mutex_unlock(&pool->lock);
        
        // 调用你的请求处理函数
        printf("线程 %lu 开始处理客户端 fd=%d\n", pthread_self(), client_fd);
        handle_request(client_fd);  // 你的处理函数
        close(client_fd);
        printf("线程 %lu 处理完成\n", pthread_self());
    }
    
    return NULL;
}