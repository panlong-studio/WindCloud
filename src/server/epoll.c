#include <sys/epoll.h>
#include "epoll.h"
#include "error_check.h"

void add_epoll_fd(int epfd,int fd){
    struct epoll_event evt;
    evt.data.fd=fd;
    evt.events=EPOLLIN;

    int ret=epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl add");
}

void del_epoll_fd(int epfd,int fd){
    struct epoll_event evt;
    evt.data.fd=fd;
    evt.events=EPOLLIN;

     int ret=epoll_ctl(epfd,EPOLL_CTL_DEL,fd,&evt);
    ERROR_CHECK(ret,-1,"epoll_ctl del");
}