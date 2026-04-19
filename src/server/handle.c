#include <stdio.h>       
#include <stdlib.h>        
#include <string.h>       
#include <unistd.h>       
#include <fcntl.h>       
#include <sys/types.h>    
#include <sys/stat.h>      
#include <dirent.h>       
#include <sys/socket.h>   
#include <sys/mman.h>     
#include <sys/sendfile.h> 
#include <errno.h>      
#include "handle.h"

#define SERVER_BASE_DIR "./upload"  //磁盘根目录
void send_msg(int listen_fd, const char *msg) {
    int len = strlen(msg);
if(send(listen_fd, &len, sizeof(int), MSG_NOSIGNAL) != sizeof(int)) {
        return;
    }
    if(send(listen_fd, msg, len, MSG_NOSIGNAL) != len) {
        return;
    }
}

void get_real_path(char *res, const char *path, const char *arg) {
    if (strcmp(path, "/") == 0) {
        sprintf(res, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(res, "%s%s/%s", SERVER_BASE_DIR, path, arg);
    }
}

void handle_request(int listen_fd){

    char current_path[512]="/";
    while(1){
        int cmd_len=0;
        // 第一步：接收客户端发来的命令长度
        ssize_t ret=recv(listen_fd,&cmd_len,sizeof(int),MSG_WAITALL);
        if (ret <= 0) {
            send_msg(listen_fd,"recv client cmd len failed!\n");
         break;
        }
        char cmd_buf[512]={0};
        // 第二步：根据收到的长度，接收具体的命令字符串
        ret=recv(listen_fd,cmd_buf,cmd_len,MSG_WAITALL);
        if(ret != cmd_len) {
        send_msg(listen_fd,"recv command failed");
        break;  // 接收失败，退出循环
        }
        // 第三步：解析命令字符串 (拆分出 指令 和 参数)
        char cmd[64]={0};
        char arg[256]={0};
        sscanf(cmd_buf,"%s %s",cmd,arg);

        // 第四步：根据不同的指令，调用不同的处理函数
        if(strcmp(cmd,"pwd")==0){
            handle_pwd(listen_fd,current_path);
        }else if(strcmp(cmd,"cd")==0){
            handle_cd(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"ls")==0){
            handle_ls(listen_fd,current_path);
        }else if(strcmp(cmd,"gets")==0){
            handle_gets(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"rm")==0){
            handle_rm(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"mkdir")==0){
            handle_mkdir(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"puts")==0){
            handle_puts(listen_fd,current_path,arg);
        }else{
            send_msg(listen_fd,"指令错误!\n");
            continue;
        }
    }
}
void handle_cd(int listen_fd,char *current_path,char *arg){

    if(strlen(arg)==0){
        send_msg(listen_fd,"输入错误");
        return;
    }

    if(strcmp(arg,"..")==0){ //简单实现暂时只能返回根目录
        strcpy(current_path,"/");
        send_msg(listen_fd,"已返回根目录\n");
        return;
    }

    //拼接真实路径
    char real_path[1024]={0};
    get_real_path(real_path, current_path, arg);
    // 测试这个文件夹是否存在
    DIR *dir = opendir(real_path);
    if (dir == NULL) {
        send_msg(listen_fd, "目录不存在！\n");
    } else {
        closedir(dir); // 测试成功，记得关掉

        // 更新客户端的“虚拟路径”字符串
        if (strcmp(current_path, "/") == 0) {
            sprintf(current_path, "/%s", arg);
        } else {
            strcat(current_path, "/");
            strcat(current_path, arg);
        }
        send_msg(listen_fd, "进入目录成功\n");
    }
}



void handle_ls(int listen_fd,char *current_path){
    char real_path[1024] = {0};
    sprintf(real_path, "%s%s", SERVER_BASE_DIR, current_path);
    DIR *dir;
    struct dirent *file;

    dir=opendir(real_path);
    if (dir == NULL) {
        send_msg(listen_fd,"目录打开失败\n");
        return;
    }
    // 遍历目录，把所有文件名拼接到一个大字符串里
    char result[4096]={0};
    while((file=readdir(dir))!= NULL){
        if(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0){
            continue;
        }
        strcat(result,file->d_name);
        strcat(result," ");
    }
    send_msg(listen_fd,result);
}

void handle_pwd(int listen_fd,char *current_path){
    send_msg(listen_fd,current_path);
}

void handle_rm(int listen_fd,char *current_path,char *arg){
    char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }
    if (remove(real_path) == 0) {
        send_msg(listen_fd, "删除成功\r\n");
    } else {
        send_msg(listen_fd, "删除失败\r\n");
    }
}

void handle_mkdir(int listen_fd,char *current_path,char *arg){

    char real_path[1024] = {0};
   get_real_path(real_path, current_path, arg);

    if(mkdir(real_path,0755)==0){
        send_msg(listen_fd,"创建文件夹成功\n");    
    }else{
        send_msg(listen_fd,"文件夹创建失败\n");
    }
}

void handle_gets(int listen_fd, char *current_path, char *arg){

char real_path[1024];
    // 1. 拼接服务器本地的真实路径
    get_real_path(real_path, current_path, arg);

    // 2. 以只读方式打开服务器上的文件
    int file_fd = open(real_path, O_RDONLY);
    if (file_fd == -1) {
        // 如果文件不存在
        send_msg(listen_fd, "Error: File not found on server.");
        return;
    }

    // 3. 获取服务器文件的完整信息（主要是总大小）
    struct stat st;
    fstat(file_fd, &st);
    off_t file_total_size = st.st_size;

    // 4. 向客户端发送文件总大小，让客户端知道文件有多大
    send(listen_fd, &file_total_size, sizeof(off_t), 0);

    // 5. 接收客户端发来的“断点位置”（Offset）
    // 如果是第一次下载，客户端发 0；如果是续传，客户端发已有的文件大小
    off_t offset = 0;
    if (recv(listen_fd, &offset, sizeof(off_t), MSG_WAITALL) <= 0) {
        close(file_fd);
        return;
    }

    // 6. 计算还需要发送的剩余字节数
    off_t remaining = file_total_size - offset;
    
    // 7. 使用 sendfile 实现高效传输
    // 注意：sendfile 的第三个参数 &offset 会自动从该位置开始读取，
    // 并且在发送完成后，offset 会被自动更新。
    while (remaining > 0) {
        // 参数：目标Socket, 源文件FD, 起始位置指针, 传输长度
        ssize_t sent = sendfile(listen_fd, file_fd, &offset, remaining);
        if (sent <= 0) {
            // sent == -1 且 errno == EAGAIN 表示缓冲区满
            // sent == 0 表示连接断开
            break; 
        }
        remaining -= sent; // 递减剩余任务量
    }

    close(file_fd); // 传输结束或异常，关闭文件描述符
    
}
    
    

void handle_puts(int listen_fd, char *current_path, char *arg){

    char real_path[1024];
    get_real_path(real_path, current_path, arg);

    // 1. 接收客户端告知的文件总大小
    off_t file_len = 0;
    if (recv(listen_fd, &file_len, sizeof(off_t), MSG_WAITALL) <= 0){
        send_msg(listen_fd,"recv client file len failed!\n");
        return;
    } ;

    // 2. 以读写模式打开文件（不存在则创建）
    int file_fd = open(real_path, O_RDWR | O_CREAT, 0666);
    if (file_fd == -1) {
        send_msg(listen_fd, "Error: Server failed to create file.");
        return;
    }

    // 3. 【断点检测】获取服务器本地目前已存在的文件大小
    struct stat st;
    off_t local_size = 0;
    if (fstat(file_fd, &st) == 0) {
        local_size = st.st_size;
    }

    // 4. 【协议同步】告知客户端：我这已有 local_size，你从这里开始传
    send(listen_fd, &local_size, sizeof(off_t), 0);

    // 5. 计算还需要接收的字节数
    off_t remaining = file_len - local_size;
    if (remaining <= 0) {
        printf("文件已存在且完整，无需续传。\n");
        close(file_fd);
        return;
    }

    // 6. 【核心】预展文件大小
    // mmap 必须映射有实际物理磁盘空间的文件，否则写入会报 SIGBUS 错误。
    // 我们直接将文件拉伸到最终目标大小。
    if (ftruncate(file_fd, file_len) == -1) {
    send_msg(listen_fd, "Error: Server disk full or quota exceeded.");
        close(file_fd);
        return;
    }

    // 7. 【核心】建立内存映射
    // 映射整个文件。addr=NULL(系统指定), length=总大小, prot=读写, flags=共享同步, offset=0
    char *map_ptr = mmap(NULL, file_len, PROT_READ | PROT_WRITE, MAP_SHARED, file_fd, 0);
    if (map_ptr == MAP_FAILED) {
        send_msg(listen_fd, "Error: Server memory mapping failed.");
        close(file_fd);
        return;
    }

    // 8. 【接收数据】直接将 Socket 数据收进映射区对应的偏移位置
    char *write_start = map_ptr + local_size; // 指针偏移到断点位置
    off_t received_count = 0;

    while (received_count < remaining) {
        // 直接 recv 到 mmap 映射的内存地址，省去了从用户缓冲区到内核缓冲区的拷贝
        ssize_t ret = recv(listen_fd, write_start + received_count, remaining - received_count, 0);
        if (ret <= 0) {
            // 如果连接断开，received_count 记录了本次实际收到的字节数
            send_msg(listen_fd,"传输中断，已保存当前进度。\n");
            break;
        }
        received_count += ret;
    }

    // 9. 清理工作
    munmap(map_ptr, file_len);

    // 【关键】如果是由于断开导致的停止，需将文件缩减到实际收到的大小
    // 这样下次 fstat 获取的 local_size 才是真实的“断点”
    if (received_count < remaining) {
        ftruncate(file_fd, local_size + received_count);
    } else {

       send_msg(listen_fd," ");
    }

    close(file_fd);

}
