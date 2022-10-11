#ifndef __TR9_FRAME_H__
#define __TR9_FRAME_H__

typedef struct { //原始数据帧的数据，直接指针指向的数据
    unsigned char flag_bg;
    struct {
        unsigned short cmd;
        unsigned short data_len;
        unsigned char data[1];
    } msg;
} tr9_frame_src_st;

typedef struct {          //解析过后的数据//可读性的数据
    unsigned char *frame; //原始帧数据指针
    unsigned short len;   //原始数据帧的长度

    unsigned char flag_bg;
    unsigned short cmd;
    unsigned char xor ;
    unsigned char flag_end;

    struct {
        unsigned short len;
        unsigned short bg;
        unsigned short end;
        unsigned char arr[1];
    } data;
} tr9_frame_st;

#pragma region 日志打印控制
typedef enum {
    tr9_frame_promt_dir_rx = 0,
    tr9_frame_promt_dir_tx,
    tr9_frame_promt_dir_cnt,
} tr9_frame_promt_dir_enum;

typedef struct {
    const unsigned short cmd;
    struct {
        const unsigned short lim : 9; //最大511//长度输出限制
        const unsigned short en : 1;  //使能日志输出
        const unsigned short lv : 4;  //打印等级
        const unsigned short all : 1; //数据有多少打多少
        const unsigned short cmd : 1; //命令cmd单独打印一行
    } set;
} tr9_frame_promt_set_st;
#pragma endregion 日志打印控制

#pragma region 日志打印控制
extern tr9_frame_promt_set_st const tr9_frame_promt_set_rx[];
extern tr9_frame_promt_set_st const tr9_frame_promt_set_tx[];
#pragma endregion 日志打印控制

#pragma region 日志打印控制
void tr9_frame_promt(tr9_frame_promt_dir_enum dir, tr9_frame_st *frame, const char *comment, const char *func, int line);
#pragma endregion 日志打印控制

tr9_frame_st *tr9_frame_get_and_need_free(const char *func, u8 *p, u16 p_len, int en_log);
void tr9_frame_pack2rk(unsigned short cmd, unsigned char *buf, unsigned short len);

#endif /* __TR9_FRAME_H__ */
