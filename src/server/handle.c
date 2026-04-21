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
#include "protocol.h"
#include "log.h"

#define SERVER_BASE_DIR "../test"
#define BUFFER_SIZE 4096

// 获取服务器基准目录的绝对路径，优先使用编译时指定的路径
// 如果不存在则尝试当前目录下的 test 文件夹
static const char *get_server_base_dir(void) {
    if (access(SERVER_BASE_DIR, F_OK) == 0) {
        return SERVER_BASE_DIR;
    }
    if (access("./test", F_OK) == 0) {
        return "./test";
    }
    return SERVER_BASE_DIR;
}

/**
 * 函数作用：向客户端发送一条普通文本消息
 * @param listen_fd：客户端连接 fd
 * @param msg：要发送的文本内容
 */
void send_msg(int listen_fd, const char *msg) {
    // 用于保存服务端要发回去的文本的结构体
    command_packet_t reply_packet;

    // 初始化结构体，设置命令类型为 CMD_TYPE_REPLY，内容为 msg
    init_command_packet(&reply_packet, CMD_TYPE_REPLY, msg);
    
    // 调用 send_command_packet 函数将结构体发送给客户端
    if (send_command_packet(listen_fd, &reply_packet) == -1) {
        LOG_WARN("发送响应失败，客户端fd=%d，消息=%s", listen_fd, msg);
        return;
    }

    LOG_DEBUG("响应发送成功，客户端fd=%d，消息=%s", listen_fd, msg);
}

/**
 * 函数作用：检查用户传来的路径参数是否基本合法
 * @param arg：用户输入的目录名或文件名
 * @return 合法返回 0，不合法返回 -1
 */
static int check_arg_path(const char *arg) {
    // 参数为空，或者是空字符串，都直接判为非法
    if (arg == NULL || strlen(arg) == 0) {
        return -1;
    }

    // 当前项目只做最基础的安全限制：
    // 只要出现 ..，就拒绝，避免跳出服务端根目录
    if (strstr(arg, "..") != NULL) {
        return -1;
    }

    return 0;
}

/**
 * 函数作用：根据用户输入的虚拟路径，拼接出服务器上的真实路径
 * @param res：输出参数，用来保存最终拼好的真实路径
 * @param size：res 缓冲区大小
 * @param current_path：当前虚拟路径，例如 "/" 或 "/demo"
 * @return 成功返回 0，失败返回 -1
 */
static int get_current_real_path(char *res,
                                     int size,
                                 const char *current_path) {
    // 例如：
    // current_path = /demo
    // 最终拼出来就是 ../test/demo
    const char *base_dir = get_server_base_dir();
    int ret = snprintf(res, size, "%s%s", base_dir, current_path);

    // 若snprintf 返回值 >= size，说明输出被截断了
    if (ret < 0 || ret >= size) {
        return -1;
    }

    return 0;
}

/**
 * 根据"当前虚拟路径 + 参数"，拼接出服务器上的真实路径
 * @param res 输出参数，用来保存最终拼好的真实路径
 * @param size res 缓冲区大小
 * @param path 当前虚拟路径
 * @param arg 用户输入的目录名或文件名
 * @return 成功返回 0，失败返回 -1
 */
static int get_real_path(char *res,
                          int size,
                          const char *path,
                          const char *arg) {
    // 先检查参数是否合法
    if (check_arg_path(arg) == -1) {
        return -1;
    }

    // ret 用来保存snprintf的返回值
    int ret = 0;

    const char *base_dir = get_server_base_dir();

    // 如果当前已经在根目录，就要拼接成 ../test/arg
    if (strcmp(path, "/") == 0) {
        ret = snprintf(res, size, "%s/%s", base_dir, arg);
    } else {
        // 如果当前不在根目录，就要拼接成 ../test/当前目录/arg
        ret = snprintf(res, size, "%s%s/%s", base_dir, path, arg);
    }

    // 再做一次长度检查
    if (ret < 0 || ret >= size) {
        return -1;
    }

    return 0;
}

/**
 * 函数作用：更新客户端看到的虚拟路径
 * @param current_path：当前虚拟路径，成功后会被改成新路径
 * @param size：current_path 缓冲区大小
 * @param arg：用户输入的目录名
 * @return 成功返回 0，失败返回 -1
 */
static int update_current_path(char *current_path,
                               int size,
                               const char *arg) {
    // new_path 是临时变量，先把新路径拼到这里
    // 确认没问题后再写回 current_path
    char new_path[512] = {0};
    int ret = 0;

    // 如果当前在根目录，进入 demo 后应该变为 /demo
    if (strcmp(current_path, "/") == 0) {
        ret = snprintf(new_path, sizeof(new_path), "/%s", arg);
    } eles {
        // 如果当前已经在 /demo，进入 abc 后应该变成 /demo/abc
        ret = snprintf(new_path, sizeof(new_path), "%s/%s", current_path, arg);
    }

    // 检查新路径是否过长
    if (ret < 0 || ret >= size) {
        return -1;
    }

    // 确认新路径安全后，再回写到 current_path
    strcpy(current_path, new_path);
    return 0;
}

/** 
 * 函数作用：把 command_packet_t 里的整型命令值转成枚举类型
 * @param cmd_packet：客户端发来的命令结构体
 * @return 对应的命令枚举值；如果参数为空，返回 CMD_TYPE_INVALID
 */
static cmd_type_t get_packet_cmd_type(const command_packet_t *cmd_packet) {
    // 防止空指针
    if (cmd_packet == NULL){
        return CMD_TYPE_INVALID;
    }

    // 这里本质上只是把 int 强制转换成 cmd_type_t
    return (cmd_type_t)cmd_packet->cmd_type;
}


void handle_request(int listen_fd) {
    // 每个客户端连接时都有自己独立的 current_path
    // 初始化进入连接时，默认都在虚拟根目录 /
    char current_path[512] = "/";

    // 持续循环接收这个客户端发来的命令
    while(1) {
        // cmd_packet 用来保存本轮收到的命令结构体
        command_packet_t cmd_packet;

        // 每次都按固定大小完整接收一个命令结构体
        if (recv_command_packet(listen_fd, &cmd_packet) <= 0) {
            // 返回 <=0 说明客户端断开了，或者协议接收失败
            LOG_INFO("客户端连接断开，客户端fd=%d，当前路径=%s", listen_fd, current_path);
            break;
        }

        // 取出本轮命令的枚举类型
        cmd_type_t cmd_type = get_packet_cmd_type(&cmd_packet);
        LOG_DEBUG("收到客户端命令，客户端fd=%d，命令类型=%d，数据=%s", listen_fd, cmd_type, cmd_packet.data);
    }
}