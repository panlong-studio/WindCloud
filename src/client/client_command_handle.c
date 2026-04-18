#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include "client_command_handle.h"

#define BUFFER_SIZE 4096

void process_command(int sock_fd, const char *input) {
    char cmd[100], arg[200];
    char response[4096];
    
    memset(cmd, 0, sizeof(cmd));
    memset(arg, 0, sizeof(arg));
    sscanf(input, "%s %s", cmd, arg);
    
    int len = strlen(input);
    send(sock_fd, &len, sizeof(int), 0);
    send(sock_fd, input, len, 0);
    
    if (strcmp(cmd, "gets") == 0) {
        if (strlen(arg) == 0) {
            printf("用法: gets <文件名>\n");
            return;
        }
        
        off_t file_size = 0;
        recv(sock_fd, &file_size, sizeof(off_t), MSG_WAITALL);
        
        if (file_size < 0) {
            printf("文件不存在或为空\n");
            return;
        }
        
      // ---【新增：断点续传逻辑】---
        struct stat st;
        off_t local_size = 0;
        // 检查本地是否已有文件，获取当前大小
        if (stat(arg, &st) == 0) {
            local_size = st.st_size;
        }
        // 2. 告知服务器：我本地已经有多少了
        send(sock_fd, &local_size, sizeof(off_t), 0);

        if (local_size >= file_size) {
            printf("文件已存在且完整，无需下载。\n");
            return;
        }

        // 3. 修改：将 O_TRUNC 改为 O_APPEND (追加模式)
        int fd = open(arg, O_WRONLY | O_CREAT | O_APPEND, 0755);
        if (fd == -1) {
            perror("创建文件失败");
            return;
        }
        
        char buf[BUFFER_SIZE];
        off_t remaining = file_size - local_size; // 剩余大小更新
       
        
        while (remaining > 0) {
            int to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
            int ret = recv(sock_fd, buf, to_read, MSG_WAITALL);
            if (ret <= 0) break;
            write(fd, buf, ret);
            remaining -= ret;
        }
        
        close(fd);
        printf("下载成功: %s (%ld 字节)\n", arg, file_size);
        return;
    }
    
    if (strcmp(cmd, "puts") == 0) {
        if (strlen(arg) == 0) {
            printf("用法: puts <文件名>\n");
            return;
        }
        
        int fd = open(arg, O_RDONLY);
        if (fd == -1) {
            perror("打开文件失败");
            return;
        }
        
        struct stat st;
        fstat(fd, &st);
        off_t file_size = st.st_size;
        
        send(sock_fd, &file_size, sizeof(off_t), 0);
        // ---【新增：断点续传逻辑】---
        off_t server_offset = 0;
        // 2. 接收服务器反馈：它那边已经存了多少了
        recv(sock_fd, &server_offset, sizeof(off_t), MSG_WAITALL);

        // 3. 将本地文件指针移动到服务器要求的断点位置
        lseek(fd, server_offset, SEEK_SET);
        
        off_t remaining = file_size - server_offset; // 剩余发送量更新
        // ---【结束新增】---
        char buf[BUFFER_SIZE];
     
        while (remaining > 0) {
            int to_read = (remaining < BUFFER_SIZE) ? remaining : BUFFER_SIZE;
            int n = read(fd, buf, to_read);
            if (n <= 0) break;
            send(sock_fd, buf, n, 0);
            remaining -= n;
        }
        
        close(fd);
        printf("上传完成: %s (%ld 字节)\n", arg, file_size);
        
        int resp_len = 0;
        recv(sock_fd, &resp_len, sizeof(int), MSG_WAITALL);
        if (resp_len > 0 && resp_len < sizeof(response)) {
            recv(sock_fd, response, resp_len, MSG_WAITALL);
            response[resp_len] = '\0';
            printf("%s\n", response);
        }
        return;
    }
    
    int resp_len = 0;
    recv(sock_fd, &resp_len, sizeof(int), MSG_WAITALL);
    
    if (resp_len > 0 && resp_len < sizeof(response)) {
        recv(sock_fd, response, resp_len, MSG_WAITALL);
        response[resp_len] = '\0';
        printf("%s\n", response);
    }
}