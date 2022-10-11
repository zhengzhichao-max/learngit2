
#ifndef __MONITOR_TABLE_H__
#define __MONITOR_TABLE_H__

#include "include_all.h"

#pragma region          //重要的定义
#define ARGV_BUF_MAX 64 //ARGV_BUF_MAX//包括结束符
#define ARGV_CNT_MAX 10 //ARGV_CNT_MAX
typedef struct {
    char *argv[ARGV_CNT_MAX];
    unsigned char argc;
} argv_list_st;

// 命令列表
typedef struct {
    const char *name;
    const char *short_name;
    const char *description;
    const char *usage; //使用方法
    int (*function)(argv_list_st *argv_list);
} cmd_list_st;
#pragma endregion //重要的定义

extern const cmd_list_st g_cmd_table[];

//当有多个命令时，打印多个命令看看//cmd_name == NULL时，打印所有命令信息
void cmd_help_promt(char *cmd_name, log_level_enum log_level, const char *who, int line);

#endif /* __MONITOR_TABLE_H__ */
