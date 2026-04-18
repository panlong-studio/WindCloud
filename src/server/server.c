#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <pthread.h>
#include "queue.h"
#include "thread_pool.h"
#include "worker.h"
#include "epoll.h"
#include "server_socket.h"
#include "config.h"
#include "handle.h"
#include "error_check.h"


int pipe_fd[2];

void func(int num){
    printf("num=%d\n",num);
    write(pipe_fd[1],"1",1);
}

void load_config(char *ip, char *port) {
    get_target("ip", ip);
    printf("ip=%s\n", ip);

    get_target("port", port);
    printf("port=%s\n", port);
}

int main(){
    char ip[64] = {0}; 
    char port[64] = {0};  
	load_config(ip, port);

    pipe(pipe_fd);
    
    // 父进程，若收到sigint指令，执行func之后exit退出进程
    if(fork() != 0){
        signal(SIGINT, func);  // 修改：使用SIGINT而不是2
        wait(NULL);
        exit(0);
    }

    setpgid(0, 0);    // 父子进程分离到不同进程组

    // 定义监听fd
    int listen_fd = 0;
    init_socket(&listen_fd, ip, port);  // 需要配置文件的IP和port

    // 定义线程池，并初始化
    thread_pool_t pool;
    init_thread_pool(&pool, 5);

    // 多路复用
    int epfd = epoll_create(1);
    ERROR_CHECK(epfd, -1, "epoll_create");
    add_epoll_fd(epfd, listen_fd);
    add_epoll_fd(epfd, pipe_fd[0]);

    while(1){
        struct epoll_event lst[10];
        int nready = epoll_wait(epfd, lst, 10, -1);
        ERROR_CHECK(nready, -1, "epoll_wait");
        printf("nready=%d\n", nready);
        
        for(int idx = 0; idx < nready; idx++){  // 修改：idx++ 不是 --idx
            int fd = lst[idx].data.fd;

            if(fd == pipe_fd[0]){
                char buf[10];
                read(fd, buf, sizeof(buf));
                printf("子进程收到父进程的终止信号\n");

                pthread_mutex_lock(&pool.lock);
                pool.exitFlag = 1;
                pthread_cond_broadcast(&pool.cond);
                pthread_mutex_unlock(&pool.lock);
                
                for(int i = 0; i < pool.num; i++){  // 修改：idx改为i
                    pthread_join(pool.thread_id_arr[i], NULL);
                }
                pthread_exit((void*)NULL);
            }

            if(fd == listen_fd){
                int conn_fd = accept(listen_fd, NULL, NULL);
                ERROR_CHECK(conn_fd, -1, "accept");

                pthread_mutex_lock(&pool.lock);
                enQueue(&pool.queue, conn_fd);
                pthread_cond_signal(&pool.cond);
                pthread_mutex_unlock(&pool.lock);
            }
        }
    }

    return 0;
}
