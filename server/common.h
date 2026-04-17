#ifndef _COMMON_H_
#define _COMMON_H_

#define CMD_UPLOAD   1
#define CMD_DOWNLOAD 2
#define CODE_OK      200
#define CODE_ERR     404

#include <my_header.h>

// 下载 → 客户端 ↔ 服务端 通用结构体
typedef struct {
    int cmd;
    int code;       // 状态码 200=成功 404=没文件
    int name_len;   // 文件名长度
    char* file_name; // 文件名
    off_t file_size;    // 文件大小（off_t就是文件大小类型）
} FileMsg;



#endif