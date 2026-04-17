#include <my_header.h>
#include "socket.h"
#include "../common/file_trans.h"

int main(int argc, char *argv[])
{
    char *ip = "192.168.100.128";
    char *port = "12345";

    int sockfd = 0;
    init_socket(&sockfd, &ip, &port);

    if (argc != 3) {
        close(sockfd);
        return -1;
    }

    FileMsg msg = {0};
    strcpy(msg.file_name, argv[2]);

    // ----------------------
    // 上传：共用 send_file
    // ----------------------
    if (strcmp(argv[1], "upload") == 0) {
        msg.cmd = CMD_UPLOAD;
        send_file(sockfd, msg.file_name, &msg.file_size);
        send(sockfd, &msg, sizeof(msg), 0);
        recv(sockfd, &msg, sizeof(msg), MSG_WAITALL);
        printf("上传成功\n");
    }
    // ----------------------
    // 下载：共用 recv_file
    // ----------------------
    else if (strcmp(argv[1], "download") == 0) {
        msg.cmd = CMD_DOWNLOAD;
        send(sockfd, &msg, sizeof(msg), 0);
        recv(sockfd, &msg, sizeof(msg), MSG_WAITALL);
        recv_file(sockfd, msg.file_name, msg.file_size);
        printf("下载成功\n");
    }

    close(sockfd);
    return 0;
}