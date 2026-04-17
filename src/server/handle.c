#include<my_header.h>
#include "handle.h"

void send_response(int client_fd, const char *msg) {
    int len = strlen(msg);

    send(client_fd, &len, sizeof(int), MSG_NOSIGNAL);

    send(client_fd, msg, len, MSG_NOSIGNAL);
}

void handle_request(int listen_fd){

    char current_path[512]={0};
    while(1){
        int cmd_len=0;
        recv(listen_fd,&cmd_len,sizeof(int),MSG_WAITALL);

        char cmd_buf[512]={0};
        recv(listen_fd,cmd_buf,sizeof(cmd_buf),MSG_WAITALL);

        char cmd[50]={0};
        char arg[100]={0};
        sscanf(cmd_buf,"%s %s",cmd,arg);

        if(strcmp(cmd,"pwd")==0){
            handle_pwd(listen_fd,current_path);
        }else if(strcmp(cmd,"cd")){
            handle_cd(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"ls")){
            handle_ls(listen_fd,current_path);
        }else if(strcmp(cmd,"gets")){
            handle_gets(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"rm")){
            handle_rm(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"mkdir")){
            handle_mkdir(listen_fd,current_path,arg);
        }else if(strcmp(cmd,"puts")){
            handle_puts(listen_fd,current_path,arg);
        }
    }
}
void handle_cd(int listen_fd,char *current_path,char *arg){

    if(strlen(arg)==0){
        send_response(listen_fd,"输入错误");
        return;
    }

    if(strcmp(arg,"..")==0){
        strcpy(current_path,"/");
        send_response(listen_fd,"已返回根目录");
        return;
    }

    char real_path[1024]={0};
    if(strcmp(current_path,"/")==0){
        sprintf(real_path,"%s/%s",SERVER_BASE_DIR,arg);
    }else{
        sprintf(real_path,"%s%s/%s",SERVER_BASE_DIR,current_path,arg);
    }
    DIR *dir = opendir(real_path);
    if (dir == NULL) {
        send_response(listen_fd, "目录不存在！\n");
    } else {
        closedir(dir); // 测试成功，记得关掉

        // 更新客户端的“虚拟路径”字符串
        if (strcmp(current_path, "/") == 0) {
            sprintf(current_path, "/%s", arg);
        } else {
            strcat(current_path, "/");
            strcat(current_path, arg);
        }
        send_response(listen_fd, "进入目录成功\n");
    }
}



void handle_ls(int listen_fd,char *current_path){
    char real_path[1024] = {0};
    sprintf(real_path, "%s%s", SERVER_BASE_DIR, current_path);
    DIR *dir;
    struct dirent *file;

    dir=opendir(real_path);
    if (dir == NULL) {
        send_response(listen_fd,"目录打开失败\n");
        return;
    }
    char result[200]={0};
    while((file=readdir(dir))!= NULL){
        if(strcmp(file->d_name,".")==0 || strcmp(file->d_name,"..")==0){
            continue;
        }
        strcat(result,file->d_name);
        strcat(result," ");
    }
    send_response(listen_fd,result);
}

void handle_pwd(int listen_fd,char *current_path){
    send_response(listen_fd,current_path);
}

void handle_rm(int listen_fd,char *current_path,char *arg){
    char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }
    remove(real_path);
    int fd = open(real_path, O_RDONLY);
    if (fd == -1) {
        send_response(listen_fd, "删除成功\r\n");
    } else {
        close(fd);
        send_response(listen_fd, "删除失败\r\n");
    }
}

void handle_mkdir(int listen_fd,char *current_path,char *arg){

    char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }
    if(mkdir(real_path,0755)==-1){
        send_response(listen_fd,"创建文件夹失败\n");    
    }else{
        send_response(listen_fd,"文件夹创建成功\n");
    }
}

void handle_gets(int listen_fd, char *current_path, char *arg){

 char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }
     int file_fd = open(real_path, O_RDONLY);
     if(file_fd==-1){
         send_response(listen_fd,"文件未找到");
         return;
     }

    struct stat st;
    fstat(file_fd,&st);
    off_t file_size=st.st_size;

    send(listen_fd,&file_size,sizeof(off_t),MSG_NOSIGNAL);
    sendfile(listen_fd,file_fd,NULL,file_size);

    close(file_fd);
}
void handle_puts(int listen_fd, char *current_path, char *arg){

    off_t file_size=0;
    recv(listen_fd,&file_size,sizeof(off_t),MSG_WAITALL);

    char real_path[1024] = {0};
    if (strcmp(current_path, "/") == 0) {
        sprintf(real_path, "%s/%s", SERVER_BASE_DIR, arg);
    } else {
        sprintf(real_path, "%s%s/%s", SERVER_BASE_DIR, current_path, arg);
    }

    int file_fd=open(real_path,O_RDWR | O_CREAT | O_TRUNC,0755);
    
    ftruncate(file_fd,file_size);

    char *p=(char *)mmap(NULL,file_size,PROT_WRITE | PROT_READ, MAP_SHARED,file_fd,0);

    recv(listen_fd,p,file_size,MSG_WAITALL);
    munmap(p,file_size);
    close(file_size);


}
