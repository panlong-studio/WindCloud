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

void send_msg(int client_fd, const char *msg) {
    int len = strlen(msg);
if(send(client_fd, &len, sizeof(int), MSG_NOSIGNAL) != sizeof(int)) {
        return;
    }
    if(send(client_fd, msg, len, MSG_NOSIGNAL) != len) {
        return;
    }
}

void handle_request(int listen_fd){

    char current_path[512]="/";
    while(1){
        int cmd_len=0;
        // 第一步：接收客户端发来的命令长度
        recv(listen_fd,&cmd_len,sizeof(int),MSG_WAITALL);

        char cmd_buf[512]={0};
        // 第二步：根据收到的长度，接收具体的命令字符串
        int ret=recv(listen_fd,cmd_buf,cmd_len,MSG_WAITALL);
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
    if(strcmp(current_path,"/")==0){
        sprintf(real_path,"%s/%s",SERVER_BASE_DIR,arg);
    }else{
        sprintf(real_path,"%s%s/%s",SERVER_BASE_DIR,current_path,arg);
    }
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
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }
    if(mkdir(real_path,0755)==0){
        send_msg(listen_fd,"创建文件夹成功\n");    
    }else{
        send_msg(listen_fd,"文件夹创建失败\n");
    }
}

void handle_gets(int listen_fd, char *current_path, char *arg){

    // 1. 拼接并找到服务器上的真实文件
 char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }
    // 2. 尝试打开文件
     int file_fd = open(real_path, O_RDONLY);
     if(file_fd==-1){
         send_msg(listen_fd,"文件未找到");
         return;
     }
    // 3. 获取文件大小
    struct stat st;
    fstat(file_fd,&st);
    off_t file_size=st.st_size;
    // 4. 发送文件总大小给客户端
    send(listen_fd,&file_size,sizeof(off_t),MSG_NOSIGNAL);
    // 5. 零拷贝技术：将文件直接从内核发给网卡
    ssize_t sent=sendfile(listen_fd,file_fd,NULL,file_size);
     if(sent != file_size) {
    send_msg(lieten_fd,"sendfile failed");
}
    close(file_fd);
}
void handle_puts(int listen_fd, char *current_path, char *arg){
// 1. 客户端那边会先发来这个文件有多大，我们先接收大小
    off_t file_size=0;
    recv(listen_fd,&file_size,sizeof(off_t),MSG_WAITALL);

    // 2. 拼接服务器要保存的真实路径
    char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }

    // 3. 在服务器创建这个同名文件，准备写入
    int file_fd=open(real_path,O_RDWR | O_CREAT | O_TRUNC,0755);
    
    // 4. 将文件扩充到目标大小，防止 mmap 越界
    ftruncate(file_fd,file_size);

     // 5. 将文件映射到内存
    char *p=(char *)mmap(NULL,file_size,PROT_WRITE | PROT_READ, MAP_SHARED,file_fd,0);

    // 6. 接收网络数据，直接写进内存映射区，底层自动同步到磁盘
    recv(listen_fd,p,file_size,MSG_WAITALL);
    munmap(p,file_size);
    close(file_fd);
}
