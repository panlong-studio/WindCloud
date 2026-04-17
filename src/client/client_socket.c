#include <my_header.h>
#include "client_socket.h"

void init_socket(int* listen_fd,char* ip,char* port){
    *listen_fd=socket(AF_INET, SOCK_STREAM, 0);
    ERROR_CHECK(*listen_fd, -1, "socket");

    //绑定服务器的ip与端口号
    struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));//初始化addr

    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(ip);//本机字节序转换为网络字节序
    addr.sin_port = htons(atoi(port));

    //调用connect函数进行三次握手
    int ret = connect(*listen_fd, (struct sockaddr *)&addr, sizeof(addr));
    ERROR_CHECK(ret, -1, "connect");
}

