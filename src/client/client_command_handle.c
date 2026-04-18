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
        
        if (file_size <= 0) {
            printf("文件不存在或为空\n");
            return;
        }
        
        int fd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0755);
        if (fd == -1) {
            perror("创建文件失败");
            return;
        }
        
        char buf[BUFFER_SIZE];
        off_t remaining = file_size;
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
        
        char buf[BUFFER_SIZE];
        off_t remaining = file_size;
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