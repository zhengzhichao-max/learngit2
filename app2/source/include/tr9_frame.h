#ifndef __TR9_FRAME_H__
#define __TR9_FRAME_H__

typedef struct { //ԭʼ����֡�����ݣ�ֱ��ָ��ָ�������
    unsigned char flag_bg;
    struct {
        unsigned short cmd;
        unsigned short data_len;
        unsigned char data[1];
    } msg;
} tr9_frame_src_st;

typedef struct {          //�������������//�ɶ��Ե�����
    unsigned char *frame; //ԭʼ֡����ָ��
    unsigned short len;   //ԭʼ����֡�ĳ���

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

#pragma region ��־��ӡ����
typedef enum {
    tr9_frame_promt_dir_rx = 0,
    tr9_frame_promt_dir_tx,
    tr9_frame_promt_dir_cnt,
} tr9_frame_promt_dir_enum;

typedef struct {
    const unsigned short cmd;
    struct {
        const unsigned short lim : 9; //���511//�����������
        const unsigned short en : 1;  //ʹ����־���
        const unsigned short lv : 4;  //��ӡ�ȼ�
        const unsigned short all : 1; //�����ж��ٴ����
        const unsigned short cmd : 1; //����cmd������ӡһ��
    } set;
} tr9_frame_promt_set_st;
#pragma endregion ��־��ӡ����

#pragma region ��־��ӡ����
extern tr9_frame_promt_set_st const tr9_frame_promt_set_rx[];
extern tr9_frame_promt_set_st const tr9_frame_promt_set_tx[];
#pragma endregion ��־��ӡ����

#pragma region ��־��ӡ����
void tr9_frame_promt(tr9_frame_promt_dir_enum dir, tr9_frame_st *frame, const char *comment, const char *func, int line);
#pragma endregion ��־��ӡ����

tr9_frame_st *tr9_frame_get_and_need_free(const char *func, u8 *p, u16 p_len, int en_log);
void tr9_frame_pack2rk(unsigned short cmd, unsigned char *buf, unsigned short len);

#endif /* __TR9_FRAME_H__ */
