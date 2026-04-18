#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/socket.h>
#include <errno.h>
#include "client_socket.h"
#include "config.h"
#include "client_command_handle.h"   // 新增头文件

#define BUFFER_SIZE 4096

// 封装：加载服务器配置（IP 和端口）
void load_server_config(char *ip, char *port) {
    get_target("ip", ip);
    printf("ip=%s\n", ip);

    get_target("port", port);
    printf("port=%s\n", port);
}

int main(int argc, char *argv[])
{
    char ip[64] = {0};
    char port[64] = {0};
    
    load_server_config(ip, port);

    int sock_fd = 0;
    //client_socket,客户端连接
    init_socket(&sock_fd, ip, port);

    char input[512];
    
    while (1) {
        printf("> ");
        fflush(stdout);
        
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        input[strcspn(input, "\n")] = '\0';
        
        if (strcmp(input, "quit") == 0 || strcmp(input, "exit") == 0) {
            printf("再见！\n");
            break;
        }
        
        if (strlen(input) == 0) {
            continue;
        }
       
        //client_commond_handler,服务端指令处理
        process_command(sock_fd, input);
    }
    
    close(sock_fd);
    return 0;
}