#ifndef FILE_TRANS_H
#define FILE_TRANS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <sys/socket.h>

typedef struct {
    int cmd;         // 1上传 2下载
    int code;        // 200成功 404失败
    char file_name[256];
    off_t file_size;
} FileMsg;

#define CMD_UPLOAD    1
#define CMD_DOWNLOAD  2
#define CODE_OK       200
#define CODE_ERR      404

// ========================
// 【全局共用收发函数】
// 客户端、服务端 完全通用
// ========================
void send_file(int net_fd, const char *filename, off_t *out_size);
void recv_file(int net_fd, const char *filename, off_t file_size);

#endif