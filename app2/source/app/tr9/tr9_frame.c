#include "include_all.h"

#pragma region 日志打印控制
static tr9_frame_promt_set_st const tr9_frame_promt_set_rx[] = {
    {NULL, .set.en = 0 /*0关1开, 开关非本表外的命令基础打印功能。*/, .set.cmd = 1 /*0关1开, 禁用所有打印。*/},
    {tr9_cmd_6003, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal}, //文本信息
    //{tr9_cmd_6063, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal}, //6.80 摄像头状态【RK->MCU】
    //{tr9_cmd_6064, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal}, //6.81 存储设备状态【RK->MCU】

    //6.30休眠状态位置上报时间间隔设置【RK?MCU】
    {tr9_cmd_5019, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal}, //6.81 存储设备状态【RK->MCU】

#if (MODULE_DEBUG_ota)
    {tr9_cmd_4040, .set.en = 0, .set.cmd = 1, .set.all = 0, .set.lim = 26, .set.lv = (unsigned short)log_level_fatal}, //ota
#endif
    // {tr9_cmd_6065, .set.en = 0, .set.cmd = 1, .set.all = 0, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal},//六轴数据
    // {tr9_cmd_1717, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal},

    {NULL, 0},
};

static tr9_frame_promt_set_st const tr9_frame_promt_set_tx[] = {
    {NULL, .set.en = 0 /*0关1开, 开关非本表外的命令基础打印功能。*/, .set.cmd = 1 /*0关1开, 禁用所有打印。*/},
    //{tr9_cmd_6063, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal}, //6.80 摄像头状态【RK->MCU】
    //{tr9_cmd_6064, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal},//6.81 存储设备状态【RK->MCU】

    //{tr9_cmd_1717, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal},

    //6.30休眠状态位置上报时间间隔设置【RK?MCU】
    {tr9_cmd_5019, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal},

#if (MODULE_DEBUG_ota)
    {tr9_cmd_4041, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal}, //ota
#endif
#if (MODULE_DEBUG_ic_card)
    {tr9_cmd_5013, .set.en = 1, .set.cmd = 0, .set.all = 1, .set.lim = 0, .set.lv = (unsigned short)log_level_fatal},
#endif
    {NULL, 0},
};

void tr9_frame_promt(tr9_frame_promt_dir_enum dir, tr9_frame_st *frame, const char *comment, const char *func, int line) {
    if (dir >= tr9_frame_promt_dir_cnt || (int)dir < 0) {
        return;
    }
    int en = tr9_frame_promt_set_rx[0].set.en;
    int en_all = tr9_frame_promt_set_rx[0].set.cmd;
    tr9_frame_promt_set_st const *pset = &tr9_frame_promt_set_rx[1];
    if (dir == tr9_frame_promt_dir_tx) {
        pset = &tr9_frame_promt_set_tx[1];
        en = tr9_frame_promt_set_tx[0].set.en;
        en_all = tr9_frame_promt_set_tx[0].set.cmd;
    }
    if (en_all == 0) {
        return;
    }

    const char *dir_str[] = {"rk2mcu", "mcu2rk"};
    int flgFound = -1;
    tr9_frame_st *f = frame;

    while (pset->cmd != NULL) {
        if (pset->cmd == f->cmd) {
            flgFound = 0;
            break;
        }

        pset++;
    }

    // uart1_tran_task_nms(0);
    if (flgFound == -1 && en) {
        logd("%s[0x%04X], [%s, %d]", dir_str[(int)dir], f->cmd, func, line);
    } else if (pset->set.en) {
        if (pset->set.cmd) {
            logb(pset->set.lv, "%s[0x%04X], [%s, %d]", dir_str[(int)dir], f->cmd, func, line);
        }

        unsigned short lim = (pset->set.lim < sizeof(tr9_frame_src_st) - 1) ? (sizeof(tr9_frame_src_st) - 1) : pset->set.lim;
        if (pset->set.all) {
            lim = f->len;
        }
        rk_mcu_comment_hex(dir_str[(int)dir], f->cmd, ((comment == NULL) ? "null" : comment), f->frame, f->len, f->data.bg, f->data.end, lim, (log_level_enum)pset->set.lv);
    }
    uart1_tran_task_nms(0);
}
#pragma endregion 发送数据，日志打印控制

tr9_frame_st *tr9_frame_get_and_need_free(const char *func, u8 *p, u16 p_len, int en_log) {
    if (p_len < 7) {
        if (en_log) {
            loge("err, %s", func);
        }
        return NULL;
    }

    tr9_frame_src_st *pFrameSrc = (tr9_frame_src_st *)p;

    if (pFrameSrc->flag_bg != 0xF0) {
        if (en_log) {
            loge("err, %s", func);
        }
        return NULL;
    }

    unsigned short data_len = reverse_u16(pFrameSrc->msg.data_len);

    if (data_len + 7 > p_len) {
        if (en_log) {
            loge("err, %s, (data_len + 7 > p_len), (%d + 7 > %d)", func, data_len, p_len);
        }
        return NULL;
    }

    tr9_frame_st *frame = (tr9_frame_st *)malloc(sizeof(tr9_frame_st) + data_len);
    if (frame == NULL) {
        if (en_log) {
            loge("err, %s, %d", func, sizeof(tr9_frame_st) + data_len);
        }
        return NULL;
    }

    frame->frame = p;

    //logd("len dst : %d = %d + %d", sizeof(tr9_frame_st) + data_len, sizeof(tr9_frame_st), data_len);

    frame->data.len = reverse_u16(pFrameSrc->msg.data_len);

#pragma region //xor
    frame->xor = *&pFrameSrc->msg.data[frame->data.len];
    unsigned char xorVal = _get_xor((unsigned char *)&pFrameSrc->msg, sizeof(pFrameSrc->msg) + frame->data.len - 1);
    if (xorVal != frame->xor) {
        if (en_log) {
            loge("err, %s, 0x%02X != 0x%02X", func, xorVal, frame->xor);
        }
        //putHexs_hex(p, p_len);
        free(frame);
        return NULL;
    }
#pragma endregion //xor

    frame->flag_bg = pFrameSrc->flag_bg;
    frame->cmd = reverse_u16(pFrameSrc->msg.cmd);
    frame->flag_end = *(&pFrameSrc->msg.data[frame->data.len] + 1);
    frame->len = (&pFrameSrc->msg.data[frame->data.len] + 1) - &pFrameSrc->flag_bg + 1;
    if (frame->flag_end != 0xF0) {
        if (en_log) {
            loge("err, %s", func);
        }
        free(frame);
        return NULL;
    }
    memcpy(frame->data.arr, pFrameSrc->msg.data, frame->data.len);
    frame->data.bg = pFrameSrc->msg.data - (unsigned char *)&pFrameSrc->flag_bg;
    frame->data.end = frame->data.bg + frame->data.len - 1;

    // logd(": cmd 0x%04X, data.len %d(0x%04X)", frame->cmd, frame->data.len, frame->data.len);
    // putHexs_hex(p, p_len);
    // putHexs_hex(frame->data, frame->data.len);

    return frame;
}

static unsigned short cal_cntf01_and_xor(unsigned char *valXor, unsigned char *buf, unsigned short len) {
    unsigned short i;
    unsigned short cnt = 0;

    for (i = 0; i < len; i++) {
        *valXor ^= buf[i];
        if (buf[i] == 0xF0 || buf[i] == 0xF1) {
            cnt++;
        }
    }

    return cnt;
}

static unsigned char *tr9_pack2rk_sub(unsigned char *p, unsigned char *buf, unsigned short len) {
    unsigned short i;
    unsigned char *ret = p;
    for (i = 0; i < len; i++) {
        if (buf[i] == 0xF1) {
            *ret++ = 0xF1;
            *ret++ = 0x01;
        } else if (buf[i] == 0xF0) {
            *ret++ = 0xF1;
            *ret++ = 0x02;
        } else {
            *ret++ = buf[i];
        }
    }
    return ret;
}

void tr9_frame_pack2rk(unsigned short cmd, unsigned char *buf, unsigned short len) {
    //条件未达成，不能发送到RK
    if (pub_io.b.dvr_open == 0 || s_Hi3520_Monitor.start == 0) {
        static unsigned int tic = 0;
        if (_coveri(tic) > 3) {
            tic = tick;
            loge("err%d%d", pub_io.b.dvr_open, s_Hi3520_Monitor.start);
        }
        return;
    }

#pragma region //计算增量转义字节数
    unsigned short cntF01 = 0;
    unsigned char valXor = 0x00;

    cntF01 += cal_cntf01_and_xor(&valXor, (unsigned char *)&cmd, 2);
    cntF01 += cal_cntf01_and_xor(&valXor, (unsigned char *)&len, 2);
    cntF01 += cal_cntf01_and_xor(&valXor, buf, len);
    if (valXor == 0xF0 || valXor == 0xF1) { cntF01++; }
#pragma endregion //计算增量转义字节数

#pragma region //目标数据帧超长
    if (len > 0xFFFF - 10 - cntF01) {
        loge("err, %d, %d", len, cntF01); //发的数据超长，或者，转义之后，超长
        return;
    }
#pragma endregion //目标数据帧超长

#pragma region //开辟帧缓存
    u16 len_frame_src = 1 /*head*/ + 2 /*cmd*/ + 2 /*data_len*/ + len + 1 /*xor*/ + 1 /*tail*/;
    unsigned char *frame_src = malloc(len_frame_src);
    if (frame_src == NULL) {
        loge("err");
        return;
    }
    u16 len_frame = 1 /*head*/ + 2 /*cmd*/ + 2 /*data_len*/ + len + 1 /*xor*/ + cntF01 + 1 /*tail*/;
    unsigned char *frame = malloc(len_frame);
    if (frame == NULL) {
        loge("err");
        free(frame_src);
        return;
    }
#pragma endregion //开辟帧缓存

#pragma region //数据组装
    unsigned char *p = frame_src;
    //unsigned char bufTmp[2];

#if (1)
    *p++ = 0xF0;
    _sw_endian(p, (unsigned char *)&cmd, 2);
    p += 2;
    _sw_endian(p, (unsigned char *)&len, 2);
    p += 2;
    memcpy((void *)p, (const void *)buf, len);
    p += len;
    memcpy((void *)p, (const void *)&valXor, 1);
    p += 1;
    *p++ = 0xF0;
#else
    *p++ = 0xF0;
    _sw_endian(bufTmp, (unsigned char *)&cmd, 2);
    memcpy((void *)p, (const void *)bufTmp, 2);
    p += 2;
    _sw_endian(bufTmp, (unsigned char *)&len, 2);
    memcpy((void *)p, (const void *)bufTmp, 2);
    p += 2;
    memcpy((void *)p, (const void *)buf, len);
    p += len;
    memcpy((void *)p, (const void *)&valXor, 1);
    p += 1;
    *p++ = 0xF0;
#endif
#pragma endregion //数据组装

#pragma region //debug info
#if (1)        //未被转义过
    tr9_frame_st *frame_st = tr9_frame_get_and_need_free(__func__, frame_src, len_frame_src, 1);
    if (frame_st != NULL) {
        tr9_frame_promt(tr9_frame_promt_dir_tx, frame_st, NULL, __func__, __LINE__);
        free(frame_st);
    }

    //logd("write_uart3 发送的数据长度 %d",p_len);
#if 0
    //2022-03-31
    //行驶记录数据采集命令【RK->MCU】//消息ID：0x6032
    //行驶记录数据上传【MCU->RK】//消息ID：0x6033
    if (((packet[1] == 0x60) && (packet[2] == 0x33)) || ((packet[1] == 0x60) && (packet[2] == 0x32))) {
#if 1
        logdNoNewLine("cmd=0x%02X  总包数=-%d-  当前包=-%d-  头节点=-%d-  尾节点=-%d- 发送长度 Len=%d: ", rk_6033_task.cmd,
                      rk_6033_task.packet,
                      rk_6033_task.idx + 1,
                      rk_6033_task.head,
                      rk_6033_task.tail,
                      p_len);
        tr9_show = true;

#if 0
    		{//01 00 44 00 1F 00 55 7A 00 02 9A 00 
    		   u16 k;
    		   for(k=0;k<p_len;k++)
    		    {
    		    	 logdNoTimeNoNewLine("%02X ",*(packet+k));
    		    }
    		}
#endif
        logdNoTimeNoNewLine("\r\n");
#endif
    }
#endif
#endif
#pragma endregion //debug info

#pragma region //数据组装
    p = frame;
    *p++ = 0xF0;
    p = tr9_pack2rk_sub(p, &frame_src[1], len_frame_src - 2);
    *p++ = 0xF0;
    free(frame_src);
#pragma endregion //数据组装

#if (1)                            //发送业务到RK
    write_uart3(frame, len_frame); //发送数据到串口三//MCU2RK
#endif

#if (0) //测试//转义过后的数据包
    unsigned short i = 0;
    logdNoNewLine("<");
    for (; i < len_frame; i++) {
        logdNoTimeNoNewLine("%02X ", frame[i]);
    }
    logdNoTimeNoNewLine(">\r\n");
#endif

    free(frame);
}
