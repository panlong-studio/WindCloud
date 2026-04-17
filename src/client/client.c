#include <my_header.h>
#include "client_socket.h"
#include "file_trans.h"
#include "config.h"

#define BUFFER_SIZE 4096

int main(int argc, char *argv[])
{
    char ip[64] = {0};     // 修改：分配内存而不是指针
    get_target("ip", ip);
    printf("ip=%s\n", ip);

    char port[64] = {0};   // 修改：分配内存
    get_target("port", port);  // 修改：应该是"port"不是"ip"
    printf("port=%s\n", port);

    int sock_fd = 0;
    init_socket(&sock_fd, &ip, &port);

     char input[512];
    char cmd[100], arg[200];
    char response[4096];
    
    while (1) {
        // 显示提示符
        printf("> ");
        fflush(stdout);
        
        // 读取用户输入
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // 去掉换行符
        input[strcspn(input, "\n")] = '\0';
        
        // 退出命令
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            printf("再见！\n");
            break;
        }
        
        // 跳过空行
        if (strlen(input) == 0) {
            continue;
        }
        
        // 解析命令和参数
        memset(cmd, 0, sizeof(cmd));
        memset(arg, 0, sizeof(arg));
        sscanf(input, "%s %s", cmd, arg);
        
        // ========== 发送命令长度和内容 ==========
        int len = strlen(input);
        send(sock_fd, &len, sizeof(int), 0);
        send(sock_fd, input, len, 0);
        
        // ========== gets命令：下载文件 ==========
        if (strcmp(cmd, "gets") == 0) {
            if (strlen(arg) == 0) {
                printf("用法: gets <文件名>\n");
                continue;
            }
            
            // 接收文件大小
            off_t file_size = 0;
            recv(sock_fd, &file_size, sizeof(off_t), MSG_WAITALL);
            
            if (file_size <= 0) {
                printf("文件不存在或为空\n");
                continue;
            }
            
            // 创建本地文件
            int fd = open(arg, O_WRONLY | O_CREAT | O_TRUNC, 0755);
            if (fd == -1) {
                perror("创建文件失败");
                continue;
            }
            
            // 接收文件内容
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
            continue;
        }
        
        // ========== puts命令：上传文件 ==========
        if (strcmp(cmd, "puts") == 0) {
            if (strlen(arg) == 0) {
                printf("用法: puts <文件名>\n");
                continue;
            }
            
            // 打开本地文件
            int fd = open(arg, O_RDONLY);
            if (fd == -1) {
                perror("打开文件失败");
                continue;
            }
            
            // 获取文件大小
            struct stat st;
            fstat(fd, &st);
            off_t file_size = st.st_size;
            
            // 发送文件大小
            send(sock_fd, &file_size, sizeof(off_t), 0);
            
            // 发送文件内容
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
            
            // 接收服务器响应
            int resp_len = 0;
            recv(sock_fd, &resp_len, sizeof(int), MSG_WAITALL);
            if (resp_len > 0 && resp_len < sizeof(response)) {
                recv(sock_fd, response, resp_len, MSG_WAITALL);
                response[resp_len] = '\0';
                printf("%s\n", response);
            }
            continue;
        }
        
        // ========== 其他普通命令：接收响应 ==========
        int resp_len = 0;
        recv(sock_fd, &resp_len, sizeof(int), MSG_WAITALL);
        
        if (resp_len > 0 && resp_len < sizeof(response)) {
            recv(sock_fd, response, resp_len, MSG_WAITALL);
            response[resp_len] = '\0';
            printf("%s\n", response);
        }
    }
    
    close(sock_fd);
    return 0;
}