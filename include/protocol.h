#ifndef _PROTOCOL_H
#define _PROTOCOL_H

#include <sys/types.h>

// 普通命令参数统一使用固定长度数组
#define CMD_DATA_LEN 256
// 文件名也统一使用固定长度数组
#define FILE_NAME_LEN 256

// 定义命令类型枚举
// 客户端在发送命令时会写入这个编号
// 服务器在接收命令时会根据这个编号来判断命令类型
typedef enum {
    CMD_TYPE_INVALID = 0, // 无效命令
    CMD_TYPE_PWD,         // 查看当前虚拟路径
    CMD_TYPE_CD,          // 切换虚拟目录
    CMD_TYPE_LS,          // 列出当前虚拟目录下的文件和目录
    CMD_TYPE_GETS,        // 下载文件
    CMD_TYPE_PUTS,        // 上传文件
    CMD_TYPE_RM,          // 删除文件
    CMD_TYPE_MKDIR,       // 创建目录
    CMD_TYPE_REPLY        // 服务端返回的普通文本响应
}cmd_type_t;

// 普通命令结构体
// 客户端发送命令时会将命令类型、参数长度和参数内容封装成结构体发送给服务器
typedef struct {
    int cmd_type; // 命令类型，使用上面定义的枚举
    int data_len; // 命令参数的实际长度，单位是字节
    char data[CMD_DATA_LEN]; // 命令参数，或者普通文本响应（固定长度数组）
} command_packet_t;

// 文件传输结构体
// 用于文件上传和下载的命令，包含文件名、文件大小和当前传输位置等信息
typedef struct {
    int cmd_type; // 命令类型（puts/gets）
    int data_len; // 文件名长度（file_name 中有效的字符串长度）
    char file_name[FILE_NAME_LEN]; // 文件名（固定长度数组）
    off_t file_size; // 文件总大小
    off_t offset; // 文件偏移量，表示当前传输到文件的哪个位置了，用于断点续传
} file_packet_t;

/* 
 * 函数作用：把命令字符串转换成命令枚举值
 * 参数 cmd_str：例如 "pwd"、"ls"、"puts"
 * 返回值：成功时返回对应的 cmd_type_t，失败返回 CMD_TYPE_INVALID
 */
cmd_type_t get_cmd_type(const char *cmd_str);

/*
 * 函数作用：将缓冲区中的数据完整发送到文件描述符对应的套接字
 * 参数 fd：socket 文件描述符
 * 参数 buf：要发送的数据缓冲区的起始地址
 * 参数 len：要发送的数据长度（单位：字节）
 * 返回值：成功返回 0，失败返回 -1
 */
int send_full(int fd, const void *buf, int len);

/*
 * 函数作用：接收数据并将其存储在缓冲区中，直到接收到指定长度的数据
 * 参数 fd：socket 文件描述符
 * 参数 buf：用于存储接收数据的缓冲区的起始地址
 * 参数 len：要接收的数据长度（单位：字节）
 * 返回值：成功时返回实际接收到的字节数，失败返回 <=0
 */
int recv_full(int fd, void *buf, int len);

/*
 * 函数作用：初始化命令结构体
 * 参数 packet：要初始化的命令包结构体指针
 * 参数 cmd_type：命令类型枚举值
 * 参数 data：命令参数字符串或者普通文本消息
 */
void init_command_packet(command_packet_t *packet, cmd_type_t cmd_type, const char *data);

/* 
 * 函数作用：初始化文件传输结构体
 * 参数 packet：要初始化的文件传输包结构体指针
 * 参数 type：命令类型，一般是 CMD_TYPE_PUTS 或 CMD_TYPE_GETS
 * 参数 file_name：文件名，可以传 NULL
 * 参数 file_size：文件总大小
 * 参数 offset：断点续传位置
 * 返回值：无
 */
int init_file_packet(file_packet_t *packet, cmd_type_t type, const char *file_name, off_t file_size, off_t offset);

/* 
 * 函数作用：发送命令结构体到服务器
 * 参数 fd：socket 文件描述符
 * 参数 packet：要发送的命令结构体指针
 * 返回值：成功返回 0，失败返回 -1
 */
int send_command_packet(int fd, const command_packet_t *packet);

/* 
 * 函数作用：从服务器接收命令结构体
 * 参数 fd：socket 文件描述符
 * 参数 packet：用于存储接收数据的命令结构体指针
 * 返回值：成功时返回接收字节数，失败返回 <=0
 */
int recv_command_packet(int fd, command_packet_t *packet);

/* 
 * 函数作用：发送文件传输结构体
 * 参数 fd：socket 文件描述符
 * 参数 packet：要发送的文件传输结构体指针
 * 返回值：成功返回 0，失败返回 -1
 */
int send_file_packet(int fd, const file_packet_t *packet);

/* 
 * 函数作用：接收文件传输结构体
 * 参数 fd：socket 文件描述符
 * 参数 packet：用于存储接收数据的文件传输结构体指针
 * 返回值：成功时返回接收字节数，失败返回 <=0 或 -1
 */
int recv_file_packet(int fd, file_packet_t *packet);

#endif
