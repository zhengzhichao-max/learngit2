
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "include_all.h"

#if (0)
#include "monitor_table.h"
#include "monitor.h"
#include "clib.h"
#include "logger.h"
#endif

#define DEBUG_HERE_202209171509 0
#define IMPORTANT_BUT_NEED_DESIGN 0

#if (0)        //�û����Թ��ܿ��صȵȣ�����
#pragma region //�û����Թ��ܿ��صȵȣ�����
#define CMD_TEMP
#define CMD_GNSS
#define RDSS_GNSS
#define CMD_RDSS_CMD
#define CMD_Bat
#define CMD_SOS
#define CMD_FACTORY
#pragma endregion //�û����Թ��ܿ��صȵȣ�����
#endif            //�û����Թ��ܿ��صȵȣ�����

//ret:argc val//Լ��strΪ�����ַ�����"abc"�����֣���'\0'������
static unsigned char str_to_argv(char *str, argv_list_st *argv_list);

static char *get_cmd_string_need_free(unsigned char *arr, unsigned short len);
static void argv_promt(argv_list_st *argv_list, const char *who, int line);
static const cmd_list_st *find_cmd(const char *cmd_name, unsigned char *cnt_cmd, unsigned char *cnt_cmd_short);

//*#############################################################################
//*�� �� �� ��
//*��    �� ��
//*˵    �� ��
//*��    �� ��
//*�� �� ֵ ��
//*��ʷ��¼ ��
//*״    ̬ ��
//*#############################################################################
//ret:argc val//Լ��strΪ�����ַ�����"abc"�����֣���'\0'������
static unsigned char str_to_argv(char *str, argv_list_st *argv_list) {
    int len = strlen(str);
    if (len + 1 >= ARGV_BUF_MAX) {
        loge("err");
        return 0;
    }

#if (DEBUG_HERE_202209171509)
    char *str_tmp = (char *)malloc(len + 1);
    if (str_tmp == NULL) {
        loge("err");
        return 0;
    }
    sprintf(str_tmp, str);
#endif

    unsigned char i;
    unsigned char has_argv = 0;
    memset(argv_list, 0, sizeof(argv_list_st));
    for (i = 0; i < ARGV_BUF_MAX; ++i) {
        if (str[i] == '\0') {
            break;
        } else if (str[i] == ' ') {
            str[i] = '\0';

            if (has_argv) {
                has_argv = 0;

                if (argv_list->argc >= ARGV_CNT_MAX) {
                    loge("err, argc %d > %d", argv_list->argc, ARGV_CNT_MAX);
                    break;
                }
            }
        } else {
            if (!has_argv) {
                has_argv = 1;
                argv_list->argv[argv_list->argc++] = &str[i];
            }
        }
    }

#if (DEBUG_HERE_202209171509)
    logd_NoNewLn("<%s> argc %d <", str_tmp, argv_list->argc);
    free(str_tmp);
    for (i = 0; i < argv_list->argc - 1; i++) {
        logb_empty(log_level_debug, "%s,", argv_list->argv[i]);
    }
    logb_endColor_newLn(log_level_debug, "%s>", argv_list->argv[i]);
#endif //#ifdef DEBUG_HERE_202209171509

    return argv_list->argc;
}

//*#############################################################################
//*�� �� �� ��find_cmd
//*��    �� ���������������Ƿ��ڱ���
//*˵    �� ��
//*��    �� ��
//*�� �� ֵ ��
//*��ʷ��¼ ��
//*״    ̬ ��
//*#############################################################################
//NULL//��NULL
static const cmd_list_st *find_cmd(const char *cmd_name, unsigned char *cnt_cmd, unsigned char *cnt_cmd_short) {
    const cmd_list_st *cmd_first = NULL;
    const cmd_list_st *cmd_short_first = NULL;
    const cmd_list_st *cmd;

#if (DEBUG_HERE_202209171509)
    logd("cmd_name<%s>", cmd_name);
#endif

    *cnt_cmd = 0;
    *cnt_cmd_short = 0;

    // ��������
    for (cmd = g_cmd_table; cmd != NULL && cmd->name != NULL; ++cmd) {
        // ��ֻ�Ǹ�ǰ׺
        if (_strnicmp(cmd_name, cmd->name, strlen((const char *)cmd_name)) == 0) {
            (*cnt_cmd)++;
        }

        // ��ֻ�Ǹ�ǰ׺
        if (_strnicmp(cmd_name, cmd->short_name, strlen((const char *)cmd_name)) == 0) {
            (*cnt_cmd_short)++;
        }

        // ��ȫ�Ǻ�
        if (stricmp(cmd_name, cmd->name) == 0) {
            if (cmd_first == NULL) {
                cmd_first = cmd;
            }
        }

        // ��ȫ�Ǻ�
        if (stricmp(cmd_name, cmd->short_name) == 0) {
            if (cmd_short_first == NULL) {
                cmd_short_first = cmd;
            }
        }
    }

    if (*cnt_cmd == 1) { // ����Ψһ�ĳ�����
        if (cmd_first != NULL) {
            return cmd_first;
        }
#if (0)
        else {
            return NULL;
        }
#endif
    }

    if (*cnt_cmd > 1) { // �ж�����ϵĳ�����
        return NULL;
    }

    if (*cnt_cmd_short == 1) { // ����Ψһ�Ķ�����
        if (cmd_short_first != NULL) {
            return cmd_short_first;
        }
#if (0)
        else {
            return NULL;
        }
#endif
    }

    // һ����������û���ҵ�
    // �ж�����ϵĳ�����

    return NULL;
}

static void argv_promt(argv_list_st *argv_list, const char *who, int line) {
    if (argv_list->argc > 0) {
        log_level_enum log_level = log_level_debug;

        logb_NoNewLn(log_level, "[%s, %d] cmd[%d]: {%s", who, line, argv_list->argc, CSI_END);
        logb_bgEndColor(log_level_fatal, "%s", argv_list->argv[0]);
        logb_bgColor(log_level, );

        if (argv_list->argc > 1) {
            int i = 1;
            for (; i < argv_list->argc;) {
                logb_empty(log_level, ", %s", argv_list->argv[i++]);
            }
        }

        logb_endColor_newLn(log_level, "}");
    } else {
        loge("null");
    }
}

//ret: ����ָ�룬��Ҫfree//�����ޡ������ַ���ͷ��β��������
static char *get_cmd_string_need_free(unsigned char *arr, unsigned short len) {
    unsigned short len_head = strlen(CMD_BEGIN_WITH);
    if (len <= (len_head + strlen(CMD_END_WITH))) {
        return NULL;
    }

    // ��ֻ�Ǹ�ǰ׺
    if (_strnicmp((const char *)arr, CMD_BEGIN_WITH, len_head) != 0) {
        return NULL;
    }

    unsigned short len_str = len - len_head;
    char *str = (char *)malloc(len_str + 1);
    if (str == NULL) {
        return NULL;
    }

    memcpy((void *)str, (const void *)(arr + len_head), len_str);
    str[len_str] = '\0';

    char *pgrgn = strstr((const char *)str, CMD_END_WITH);
    if (pgrgn == NULL || pgrgn == str) {
        free(str);
        return NULL;
    }

    *pgrgn = '\0';

    return str;
}

//ret: argc // < 0 û�н��������� // = 0 �������У���δ���������� // >= 1 ������
int cmd_handle(unsigned char *arr, unsigned short len) {
    char *str = get_cmd_string_need_free(arr, len);

    if (str == NULL) {
        return -1;
    }

#if (DEBUG_HERE_202209171509 && 0)
    cmd_help_promt(NULL, log_level_debug, __func__, __LINE__);
#endif

    //logd("cmd<%s>", str);
    argv_list_st argv_list;
    if (str_to_argv(str, &argv_list)) { // ��g_cmd_buf������ת��Ϊ��������б�
        char *cmd_name = argv_list.argv[0];

        if (cmd_name != NULL) {
            unsigned char cnt_cmd = 0;
            unsigned char cnt_cmd_short = 0;

            // ��������
            const cmd_list_st *cmd = find_cmd(cmd_name, &cnt_cmd, &cnt_cmd_short);

            // ִ������
            if (cmd != NULL) { //�ҵ���ȫƥ�������//������򣬶�����
                argv_promt(&argv_list, __func__, __LINE__);
                // cmd_help_promt(cmd_name, log_level_info, __func__, __LINE__);
                cmd->function(&argv_list);
            } else {
                if (cnt_cmd > 1 || cnt_cmd_short > 1) { // �ж�����������һ����Ϊǰ׺��
                    logw("cnt cmd: %d, %d, multi cmd", cnt_cmd, cnt_cmd_short);
                    cmd_help_promt(cmd_name, log_level_warn, __func__, __LINE__);
                } else {
                    loge("cnt cmd: %d, %d, cmd{%s} not found", cnt_cmd, cnt_cmd_short, cmd_name);
                    cmd_help_promt(cmd_name, log_level_err, __func__, __LINE__);
                }
            }
        }
#if (DEBUG_HERE_202209171509)
        else {
            loge("no cmd");
        }
#endif
    }
#if (DEBUG_HERE_202209171509)
    else {
        loge("no cmd");
    }
#endif

    free(str);
    return argv_list.argc;
}
