#include <my_header.h>
#include <strings.h>
#include "../../include/config.h"

int get_target(char *key, char *value){
    FILE * file = fopen("../../config/config.ini","r");
    while(1){
        char line[100];
        bzero(line,sizeof(line));

        char *res = fgets(line,sizeof(line),file);
        if(res == NULL){
            char buf[] = "NOTHING IN HERE \n";
            memcpy(value,buf,strlen(buf));
            return -1;
        }

        char *line_key = strtok(line,"=");
        if(strcmp(key,line_key) == 0){
            char *line_value = strtok(NULL,"=");
            memcpy(value, line_value, strlen(line_value));
            return 0;
        }
    }
    return 0;
}

/* 使用方法 */
/* int main(){ */
/*     char target[100]; */
/*     bzero(target,sizeof(target)); */
/*     get_target("ip",target); */
/*     printf("ip=%s\n",target); */
/* /1* 可以不用重新设置，直接清空数组即可 *1/ */
/*     bzero(target,sizeof(target)); */
/*     get_target("port",target); */
/*     printf("port=%s\n",target); */

/* } */
