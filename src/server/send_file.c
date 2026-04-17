#include <my_header.h>
#include "sys/sendfile.h"
#include "send_file.h"
#include <errno.h>
#include "common.h"


void  snend_file(int fd,char* file_name){
    int file_fd=open(file_name,O_RDWR);

    //初始化FileMsg结构体
    FileMsg msg;
    memset(&msg,0,sizof(msg));   //置零

    //若不能打开客户端请求的file_name文件，msg.code置为404
	if(file_fd<0){
        msg.code=404;
        send(fd,&msg,sizeof(msg),MSG_NOSIGNAL);
    }
    ERROR_CHECK(file_fd,-1,"open");   //服务器端终止进程
    
    msg.file_name=file_name;   //file_name加入msg
    struct stat st;
    fstat(file_fd,&st);
    msg.file_size=st.st_size;   //file_size加入msg

    msg.name_len=strlen(file_name);  //name_len加入msg

    //发送msg(数据头)
    send(file_fd,&msg,sizeof(msg),MSG_NOSIGNAL);

    off_t offset=0;    //文件发送偏移量，防止一次传输数据传不完
    while(1){
       int ret=senfile(fd,file_fd,&offset,msg.file_size-offset);
       if(ret>0)   continue;        //没发完文件，continue继续（循环）发
       if(ret==0)  break;           //文件发完，break退出循环
       if(errno==EINTR)   continue; //被信号打断，继续重试
       break;                       //防止保险
    }

    close(file_fd);
}