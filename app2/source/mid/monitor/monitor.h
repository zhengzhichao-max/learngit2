#ifndef __MONITOR_H__
#define __MONITOR_H__

#include "logger.h"

#pragma region //��Ҫ�Ķ���
#pragma region //�����������������ڵ�string;//��������ͷ������β
//cmd{#cmd< help a 1 2 bs\r\n}
//cmd{#cmd< hp a 1 2 bs\r\n}
//cmd{#cmd<  reboot a 1 2 bs  \r\n}
//cmd{#cmd<   res a 1 2 bs  \r\n}
#define CMD_BEGIN_WITH "#cmd<"
#define CMD_END_WITH "\r\n"
#pragma endregion //�����������������ڵ�string;//��������ͷ������β

#pragma region // ������Ϣ����
#define COMMAND_NAME "Command name"
#define SHORT_NAME "Short name"
#define DESCRIPTION "Description"
#define USAGE "Usage"

#define SIZE_COMMAND_NAME (12 + 4)
#define SIZE_SHORT_NAME (10 + 4)
#define SIZE_DESCRIPTION (25 + 4)
#pragma endregion // ������Ϣ����
#pragma endregion //��Ҫ�Ķ���

int cmd_handle(unsigned char *arr, unsigned short len);

#endif /* __MONITOR_H__ */

#if (0) //δʹ��
// #define CMD_START_POS -1
// #define CMD_MAX_POS (ARGV_BUF_MAX - 2)
// #define CMD_PROMPT "# "
// ----------------------------------
// #define KEY_BACKSPACE 0x8
// #define KEY_ENTER 0xD
// #define PRINT_BACKSPACE() printf("%c %c", KEY_BACKSPACE, KEY_BACKSPACE)
// #define INVALID_COMMAND NULL
// #define AMBIGUOUS_COMMAND ((ARGV_BUF_MAX *)-1)
// #define COMMAND_HANDLE_DELAY 50
//-------------------------------USER COM define-------------------------//
// #define FLAG_EXTENDCOM_INPUT 0
// #define FLAG_WIFI_INPUT 1
// #define FLAG_RDSS_COM (1 << 0)
// #define FLAG_GNSS_COM (1 << 1)
// #define FLAG_RDGN_COM (FLAG_RDSS_COM | FLAG_GNSS_COM)
// #define USERCOM_RECV_BUF_LEN 128
#endif //δʹ��
