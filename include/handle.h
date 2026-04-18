#ifndef _HANDLE_H_
#define _HANDLE_H_


void send_msg(int client_fd, const char *msg);// 发送到客户端

void handle_request(int listen_fd);  //相当于worker 判断逻辑都在里面，可以直接调用

void handle_cd(int listen_fd,char *current_path,char *arg);
void handle_ls(int listen_fd,char *current_path);
void handle_pwd(int listen_fd,char *current_path);
void handle_rm(int listen_fd,char *current_path,char *arg);
void handle_mkdir(int listen_fd,char *current_path,char *arg);
void handle_gets(int listen_fd, char *current_path, char *arg);
void handle_puts(int listen_fd, char *current_path, char *arg);
#endif /* _\=filename_H_ */

