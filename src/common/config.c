#include <strings.h>
#include <stdio.h>
#include <string.h>
#include "../../include/config.h"


int get_target(char *key, char *value) {
    FILE *file = fopen("../../config/config.ini", "r");
    if (file == NULL){
        printf("config.ini is NULL\n");
        return -1;
    } 

    char line[100];
    while (fgets(line, sizeof(line), file)) {
        // 1. 去掉换行符（这步必须在前面，方便后续判断）
        line[strcspn(line, "\r\n")] = '\0';

        // 2. 核心：处理空行
        // 如果去掉换行符后长度为0，说明是纯空行；
        // 如果第一个字符是空格，可能也是无效行（进阶可以处理空白符）。
        if (line[0] == '\0') {
            continue; // 直接跳过，进入下一次循环
        }

        // 3. 处理注释行（习惯上以 # 或 ; 开头）
        if (line[0] == '#' || line[0] == ';') {
            continue;
        }

        // 4. 解析逻辑
        char *line_key = strtok(line, "=");
        if (line_key != NULL && strcmp(key, line_key) == 0) {
            char *line_value = strtok(NULL, "=");
            if (line_value != NULL) {
                strcpy(value, line_value);
                fclose(file);
                return 0;
            }
        }
    }

    fclose(file);
    return -1;
}
/* 使用方法 */
/* int main(){ */
/*     char ip[100]={0}; */
/*     char port[100]={0}; */

/*     get_target("ip",ip); */
/*     printf("ip=%s\n",ip); */
/* } */
