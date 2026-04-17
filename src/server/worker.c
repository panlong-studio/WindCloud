#include <my_header.h>
#include "thread_pool.h"
#include "../common/file_trans.h"

void *worker(void *arg) {
    thread_pool_t *pool = (thread_pool_t *)arg;

    while (1) {
        pthread_mutex_lock(&pool->lock);
        while (isEmpty(&pool->queue) && !pool->exitFlag) {
            pthread_cond_wait(&pool->cond, &pool->lock);
        }
        if (pool->exitFlag) {
            pthread_mutex_unlock(&pool->lock);
            pthread_exit(NULL);
        }

        int conn_fd = deQueue(&pool->queue);
        pthread_mutex_unlock(&pool->lock);

        // ----------------------
        // 接收命令
        // ----------------------
        FileMsg msg;
        recv(conn_fd, &msg, sizeof(msg), MSG_WAITALL);

        if (msg.cmd == CMD_UPLOAD) {
            // 【共用：收文件】
            recv_file(conn_fd, msg.file_name, msg.file_size);
            msg.code = CODE_OK;
            send(conn_fd, &msg, sizeof(msg), 0);
        }
        else if (msg.cmd == CMD_DOWNLOAD) {
            // 【共用：发文件】
            send_file(conn_fd, msg.file_name, &msg.file_size);
            msg.code = CODE_OK;
            send(conn_fd, &msg, sizeof(msg), 0);
        }

        close(conn_fd);
    }
    return NULL;
}