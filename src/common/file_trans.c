#include "../../include/file_trans.h"

// ==============================================
// 【通用发送文件】
// 客户端上传、服务端下载 都用这一个函数！
// ==============================================
void send_file(int net_fd, const char *filename, off_t *out_size)
{
    int fd = open(filename, O_RDONLY);
    if (fd < 0) {
        *out_size = -1;
        return;
    }

    struct stat st;
    fstat(fd, &st);
    *out_size = st.st_size;

    char *p = mmap(NULL, st.st_size, PROT_READ, MAP_PRIVATE, fd, 0);
    send(net_fd, p, st.st_size, MSG_NOSIGNAL);
    munmap(p, st.st_size);
    close(fd);
}

// ==============================================
// 【通用接收文件】
// 服务端收上传、客户端收下载 都用这一个函数！
// ==============================================
void recv_file(int net_fd, const char *filename, off_t file_size)
{
    int fd = open(filename, O_RDWR | O_CREAT | O_TRUNC, 0600);
    ftruncate(fd, file_size);

    char *p = mmap(NULL, file_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    recv(net_fd, p, file_size, MSG_WAITALL);
    munmap(p, file_size);
    close(fd);
}
