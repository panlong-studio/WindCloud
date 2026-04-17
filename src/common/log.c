#include "log.h"

static int g_log_level=LOG_LEVEL_INFO; //将全局级别初始化为默认级别
static FILE* g_log_fp=NULL;//日志文件初始化为NULL
static pthread_mutex_t g_log_mutex = PTHREAD_MUTEX_INITIALIZER;//线程安全锁

//将字符串级别转换为整数级别 用于从用户输入的字符串级别转换为内部使用的整数级别 
//或者从配置文件读取的字符串级别转换为整数级别
static int level_to_int(const char* level_str){
    if(strcasecmp(level_str,"DEBUG")==0) return LOG_LEVEL_DEBUG;
    if(strcasecmp(level_str,"INFO")==0)  return LOG_LEVEL_INFO;
    if(strcasecmp(level_str,"WARN")==0)  return LOG_LEVEL_WARN;
    if(strcasecmp(level_str,"ERROR")==0) return LOG_LEVEL_ERROR;
    return LOG_LEVEL_INFO;//默认级别
}

//获取级别对应的描述文字 用于将级别写入日志文件
static const char* level_to_name(int level){
    switch(level){
        case LOG_LEVEL_DEBUG: return "DEBUG";
        case LOG_LEVEL_INFO:  return "INFO";
        case LOG_LEVEL_WARN:  return "WARN";
        case LOG_LEVEL_ERROR: return "ERROR";
        default: return "UNKNOWN";
    }
}

//初始化日志系统
//----level_str 初始日志级别字符串 (如 "DEBUG", "INFO" 等)
//----log_file  日志保存路径，若为 NULL 则输出到控制台
int init_log(const char* level_str, const char* log_file){
    if(level_str){
        g_log_level=level_to_int(level_str);//level_str为NULL时 返回的是默认级别
    }
    
    if(log_file){
        g_log_fp=fopen(log_file,"a");//以追加模式打开日志文件
        if(!g_log_fp){
            perror("Failed to open log file");
            return -1;
        }
    }else{
        g_log_fp=stdout;//如果没有指定日志文件，则输出到控制台
    }
    return 0;
}


void close_log(){
    //当前日志文件指针不为NULL（即确实成功打卡了日志文件）
    //且不为标准输出时(即用户指定了日志文件)，才需要关闭文件
    if(g_log_fp && g_log_fp!=stdout){
        fclose(g_log_fp);//关闭日志文件
    }
    pthread_mutex_destroy(&g_log_mutex);//销毁互斥锁
}


void log_write(int level, const char* file, int line, const char* func, const char* fmt, ...){
    //1.级别检查：如果当前级别低于系统设定级别，则直接返回
    if(level<g_log_level){
        return;
    }
    
    //当前级别高于系统设定级别，继续处理日志写入
    //2.获取当前系统时间
    time_t now=time(NULL);//获取当前时间戳
    struct tm* tm_info=localtime(&now);//将时间戳转换为本地时间结构
    char time_str[64];//用于存储格式化后的时间字符串
    strftime(time_str,sizeof(time_str),"%Y-%m-%d %H:%M:%S",tm_info);//将时间结构格式化为字符串
    
    //3.格式化日志消息
    char log_msg[1024];//用于存储格式化后的日志消息
    va_list args;//定义可变参数列表
    va_start(args,fmt);//初始化可变参数列表，fmt是最后一个固定参数
    vsnprintf(log_msg,sizeof(log_msg),fmt,args);//将可变参数格式化为日志消息字符串
    va_end(args);//结束可变参数处理
    
    //4.线程安全写入日志
    pthread_mutex_lock(&g_log_mutex);
    //日志格式：[时间] [级别] [文件:行号 函数名] 消息内容
    fprintf(g_log_fp,"[%s] [%s] [%s:%d %s] %s\n",time_str,level_to_name(level),file,line,func,log_msg);
    fflush(g_log_fp);//确保日志立即写入文件
    fprintf(g_log_fp,"\n");//写入换行符 分隔日志条目
    pthread_mutex_unlock(&g_log_mutex);//解锁

}

               
