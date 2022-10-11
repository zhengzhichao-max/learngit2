
#ifndef __MONITOR_TABLE_H__
#define __MONITOR_TABLE_H__

#include "include_all.h"

#pragma region          //��Ҫ�Ķ���
#define ARGV_BUF_MAX 64 //ARGV_BUF_MAX//����������
#define ARGV_CNT_MAX 10 //ARGV_CNT_MAX
typedef struct {
    char *argv[ARGV_CNT_MAX];
    unsigned char argc;
} argv_list_st;

// �����б�
typedef struct {
    const char *name;
    const char *short_name;
    const char *description;
    const char *usage; //ʹ�÷���
    int (*function)(argv_list_st *argv_list);
} cmd_list_st;
#pragma endregion //��Ҫ�Ķ���

extern const cmd_list_st g_cmd_table[];

//���ж������ʱ����ӡ��������//cmd_name == NULLʱ����ӡ����������Ϣ
void cmd_help_promt(char *cmd_name, log_level_enum log_level, const char *who, int line);

#endif /* __MONITOR_TABLE_H__ */
