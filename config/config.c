#include <my_header.h>
#include <strings.h>
#include "config.h"

int get_target(char *key, char *value){
    FILE * file = fopen("config.ini","r");
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

