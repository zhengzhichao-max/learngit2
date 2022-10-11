
#include "include_all.h"

// #pragma pack(1)
// typedef struct {
//     u16 len;
//     unsigned char arr[1];
// } data_st;
// #pragma pack()

// static data_st *get_tr9_frame(u16 cmd) {
//     return NULL;
// }

// static data_st *get_tr9_frame_cmd(u8 *buf, u16 len) {
//     return NULL;
// }

// unsigned char *pCmd = (unsigned char *)&cmd;
// union {
//     unsigned char arr[2];
//     unsigned short val;
// } buff16 = {.arr[0] = pCmd[1], .arr[1] = pCmd[0]};

#if (0)
static void pack_tr9_data_2xxx(unsigned short cmd, unsigned char *buf, unsigned short len) {
    if (pub_io.b.dvr_open == 0 || s_Hi3520_Monitor.start == 0) {
        static unsigned int tic = 0;
        if (_coveri(tic) > 3) {
            tic = tick;
            loge("err, %d, %d", pub_io.b.dvr_open, s_Hi3520_Monitor.start);
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
    u16 len_frame = 1 /*head*/ + 2 /*cmd*/ + 2 /*data_len*/ + len + 1 /*xor*/ + cntF01 + 1 /*tail*/;
    unsigned char *frame = malloc(len_frame);
    if (frame == NULL) {
        loge("err");
        return;
    }
#pragma endregion //开辟帧缓存

#pragma region //数据组装
    unsigned char *p = frame;
    unsigned char bufTmp[2];

    *p++ = 0xF0;
    _sw_endian(bufTmp, (unsigned char *)&cmd, 2);
    p = pack_tr9_data_sub(p, bufTmp, 2);
    _sw_endian(bufTmp, (unsigned char *)&len, 2);
    p = pack_tr9_data_sub(p, bufTmp, 2);
    p = pack_tr9_data_sub(p, buf, len);
    p = pack_tr9_data_sub(p, &valXor, 1);
    *p++ = 0xF0;
#pragma endregion //数据组装

#if (0)
    write_uart3(packet, p_len); //发送数据到串口三//MCU2RK
#endif

#if (1) //测试
    unsigned short i = 0;
    logdNoNewLine("<");
    for (; i < len_frame; i++) {
        logdNoTimeNoNewLine("%02X ", frame[i]);
    }
    logdNoTimeNoNewLine(">\r\n");
#endif

#if (0) //已经被转义过，所以，tr9_frame_get_and_need_free返回值为空
    tr9_frame_st *frame = tr9_frame_get_and_need_free(__func__, packet, p_len);
    if (frame != NULL) {
        mcu2rk_hex(frame->cmd, packet, p_len, 5, 5 + frame->data_len - 1);
        free(frame);
    }
#endif

    free(frame);
}

static void pack_tr9_data_3_xxxx(unsigned short cmd, unsigned char *buf, unsigned short len) {
    if (pub_io.b.dvr_open == 0 || s_Hi3520_Monitor.start == 0) {
        static unsigned int tic = 0;
        if (_coveri(tic) > 3) {
            tic = tick;
            loge("err, %d, %d", pub_io.b.dvr_open, s_Hi3520_Monitor.start);
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
    unsigned char bufTmp[2];

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
#pragma endregion //数据组装

#if (1) //已经被转义过，所以，tr9_frame_get_and_need_free返回值为空
    tr9_frame_st *frame_st = tr9_frame_get_and_need_free(__func__, frame_src, len_frame_src);
    if (frame_st != NULL) {
        mcu2rk_hex(frame_st->cmd, frame_src, len_frame_src, 5, 5 + frame_st->data_len - 1);
        free(frame_st);
    }
#endif

#pragma region //数据组装
    p = frame;
    *p++ = 0xF0;
    p = pack_tr9_data_sub(p, &frame_src[1], len_frame_src - 2);
    *p++ = 0xF0;
    free(frame_src);
#pragma endregion //数据组装

#if (0)
    write_uart3(packet, p_len); //发送数据到串口三//MCU2RK
#endif

#if (1) //测试
    unsigned short i = 0;
    logdNoNewLine("<");
    for (; i < len_frame; i++) {
        logdNoTimeNoNewLine("%02X ", frame[i]);
    }
    logdNoTimeNoNewLine(">\r\n");
#endif

    free(frame);
}
#endif

#if (0)
//打包，发送数据到TR9
void pack_tr9_data_2(u16 cmd, u8 *buf, u16 len) {
    if (pub_io.b.dvr_open == 0 || s_Hi3520_Monitor.start == 0) {
        static unsigned int tic = 0;
        if (_coveri(tic) > 3) {
            tic = tick;
            loge("err, %d, %d", pub_io.b.dvr_open, s_Hi3520_Monitor.start);
        }
        return;
    }

    if (len > MAX_NET_PACKAGE) {
        loge("err");
        return;
    }

#pragma region //打包
#if (0)
    tr9_frame_src_st *frame = (tr9_frame_src_st *)malloc(sizeof(tr9_frame_src_st) + len + 1);
    if (frame == NULL) {
        loge("err");
        return;
    }
    frame->flag_bg = 0xF0;
    frame->msg.cmd = reverse_u16(cmd);
    frame->msg.data_len = reverse_u16(len);
    memcpy((void *)frame->msg.data, (const void *)buf, len);
    u8 *p = &frame->msg.data[len]; //xor
    *p = _get_xor((u8 *)&frame->msg, 4 + len);
    p++;
    *p = 0xF0;

    free(frame);

    packet[p_len++] = 0xF0;
    _sw_endian(packet + 1, (u8 *)&uId, 2);
    _sw_endian(packet + 3, (u8 *)&len, 2);
    p_len += 4;

    p_len += _memcpy_len(&packet[5], buf, len);
    xor1 = get_check_xor(packet, 1, p_len);
    packet[p_len++] = xor1;

    p_len += _add_tr9_char(p_len - 1, &packet[1]);

    packet[p_len++] = 0xF0;
#else
    u8 packet[MAX_NET_PACKAGE] = {0};
    u16 p_len = 0;

    packet[p_len++] = 0xF0;
    _sw_endian(packet + 1, (u8 *)&cmd, 2);
    _sw_endian(packet + 3, (u8 *)&len, 2);
    p_len += 4;

    p_len += _memcpy_len(&packet[5], buf, len);
    u8 valXor = get_check_xor(packet, 1, p_len);
    packet[p_len++] = valXor;
    logd("xor 0x%02X, len %d", valXor, len);
    putHexs_hex(buf, len);

    p_len += _add_tr9_char(p_len - 1, &packet[1]);
    logd("xor 0x%02X, len %d, %d", valXor, len, p_len);

    packet[p_len++] = 0xF0;
#endif
#pragma endregion //打包

#pragma region //debug info
    //logd("write_uart3 发送的数据长度 %d",p_len);
#if 1
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
#pragma endregion //debug info

    write_uart3(packet, p_len); //发送数据到串口三//MCU2RK

#if (0)
    logi("123");
    if (tr9_show) {
        print_TR9(packet, p_len); //发送数据到串口一 (显示作用)
    }
    logi("456");
#else
    tr9_frame_st *frame = tr9_frame_get_and_need_free(__func__, packet, p_len);
    if (frame != NULL) {
        mcu2rk_hex(frame->cmd, packet, p_len, 5, 5 + frame->data_len - 1);
        free(frame);
    }
#endif
}
#endif

void test(void) {
#if (1)
    promt_time(&sys_time, 1, log_level_fatal, __func__, __LINE__, "sys.tm", NULL);
    promt_time(&mix.time, 3, log_level_fatal, NULL, __LINE__, "mix.tm", "\tmcu boot");
#endif

#if (TEST_MCU_FLASH_20220921)
    logf("while 1");
    beep_off();
    while (1) {
        wdr();
        uart_manage_task();
    }
#endif

#if (TEST_LOGGER_MODULE)
    if (1) {
        logi("mcu");
        logw("mcu");
        loge("mcu");
        logf("mcu");

        logb(log_level_err, "abc");

        char *ret = get_build_time_need_free(str_app_build_time);
        if (ret == NULL) {
            loge("null!");
        } else {
            logf("%s", ret);
            free(ret);
        }

        while (1) {
            uart_manage_task();
        }
    }
#endif //#if (TEST_LOGGER_MODULE)

#if (TEST_MONITOR)
    if (1) {
        if (1) {
            cmd_help_promt(NULL, log_level_debug, __func__, __LINE__);
        }

        if (1) {
            char arr1[] = CMD_BEGIN_WITH " help a 1 2 bs" CMD_END_WITH;
            cmd_handle((unsigned char *)arr1, strlen(arr1));

            char arr2[] = CMD_BEGIN_WITH " hp a 1 2 bs" CMD_END_WITH;
            cmd_handle((unsigned char *)arr2, strlen(arr2));

            char arr3[] = CMD_BEGIN_WITH "  reboot a 1 2 bs  " CMD_END_WITH;
            cmd_handle((unsigned char *)arr3, strlen(arr3));

            char arr4[] = CMD_BEGIN_WITH "   res a 1 2 bs  " CMD_END_WITH;
            cmd_handle((unsigned char *)arr4, strlen(arr4));

            char arr5[] = CMD_BEGIN_WITH "  ge a 1 2 bs   " CMD_END_WITH;
            cmd_handle((unsigned char *)arr5, strlen(arr5));

            char arr6[] = CMD_BEGIN_WITH "  get a 1 2 bs" CMD_END_WITH;
            cmd_handle((unsigned char *)arr6, strlen(arr6));
        }

        if (1) {
            char arr7[] = CMD_BEGIN_WITH "   en a 1 2 bs   " CMD_END_WITH;
            cmd_handle((unsigned char *)arr7, strlen(arr7));
        }

        logb_empty(log_level_debug, "\r\n");
        logd("end");

        while (1) {
            uart_manage_task();
        }
    }
#endif //#if (TEST_MONITOR)

#if (0) //测试
    if (1) {
        const unsigned char arrtest[] = {0xF0, 0xF1, 0xF2, 0xF0, 0xF1, 0xF2, 0xF0, 0xF1, 0xF2, 0xF0};
        pub_io.b.dvr_open = s_Hi3520_Monitor.start = 1;
        u16 i = 0;
        for (; i < 10; i++) {
            putHexs_hex((unsigned char *)arrtest, i + 1);
            //pack_tr9_data2(0xF0F1, (unsigned char *)arrtest, i + 1);
            pack_tr9_data3(0xF0F1, (unsigned char *)arrtest, i + 1);
            tr9_frame_pack2rk(0xF0F1, (unsigned char *)arrtest, i + 1);
        }

        while (1) {
            uart_manage_task();
        }
    }
#elif (0) //测试 //已经被转义过，所以，tr9_frame_get_and_need_free返回值为空
    if (1) {
        const unsigned char arrtest0[] = {0xF0, 0x60, 0x65, 0x00, 0xF4, 0xF4, 0x29, 0x00, 0x00, 0x7A, 0x22, 0x20, 0x3C, 0x7A, 0x22, 0xA0, 0xBC, 0x2A, 0x1E, 0x8C, 0x3C, 0x49, 0xF4, 0x53, 0xBE, 0x27, 0x48, 0x0C, 0x3E, 0xE3, 0xCF, 0x1C, 0x41, 0x7A, 0x22, 0x20, 0x3C, 0x7A, 0x22, 0xA0, 0xBC, 0x03, 0x1C, 0x82, 0x3C, 0xA4, 0xBA, 0x52, 0xBE, 0x54, 0xAB, 0x0B, 0x3E, 0xE3, 0xCF, 0x1C, 0x41, 0x66, 0x2F, 0x5C, 0x3C, 0xC8, 0x26, 0xB4, 0xBC, 0x03, 0x1C, 0x82, 0x3C, 0xED, 0x2D, 0x55, 0xBE, 0x82, 0x0E, 0x0B, 0x3E, 0x56, 0xD2, 0x1C, 0x41, 0xC8, 0x26, 0x34, 0x3C, 0x7A, 0x22, 0xA0, 0xBC, 0x2A, 0x1E, 0x8C, 0x3C, 0xA4, 0xBA, 0x52, 0xBE, 0xE7, 0x91, 0x10, 0x3E, 0xF1, 0x02, 0xE5, 0x1C, 0x41, 0x18, 0x2B, 0x48, 0x3C, 0x18, 0x2B, 0xC8, 0xBC, 0x2A, 0x1E, 0x8C, 0x3C, 0x76, 0x57, 0x53, 0xBE, 0xF9, 0xE4, 0x0C, 0x3E, 0x97, 0xDE, 0x1C, 0x41, 0x66, 0x2F, 0x5C, 0x3C, 0x3E, 0x2D, 0xD2, 0xBC, 0x03, 0x1C, 0x82, 0x3C, 0xBE, 0xCA, 0x55, 0xBE, 0x14, 0xF5, 0x0F, 0x3E, 0x7D, 0xE3, 0x1C, 0x41, 0x7A, 0x22, 0x20, 0x3C, 0xA1, 0x24, 0xAA, 0xBC, 0x03, 0x1C, 0x82, 0x3C, 0x49, 0xF4, 0x53, 0xBE, 0x14, 0xF5, 0x0F, 0x3E, 0x7D, 0xE3, 0x1C, 0x41, 0x18, 0x2B, 0x48, 0x3C, 0xA1, 0x24, 0xAA, 0xBC, 0x03, 0x1C, 0x82, 0x3C, 0x1B, 0x91, 0x54, 0xBE, 0xCC, 0x81, 0x0D, 0x3E, 0x97, 0xDE, 0x1C, 0x41, 0x66, 0x2F, 0x5C, 0x3C, 0xA1, 0x24, 0xAA, 0xBC, 0xB6, 0x33, 0x70, 0x3C, 0x49, 0xF4, 0x53, 0xBE, 0xF9, 0xE4, 0x0C, 0x3E, 0xF1, 0x02, 0xE5, 0x1C, 0x41, 0x2A, 0x1E, 0x0C, 0x3C, 0xC8, 0x26, 0xB4, 0xBC, 0x03, 0x1C, 0x82, 0x3C, 0x76, 0x57, 0x53, 0xBE, 0x94, 0x61, 0x07, 0x3E, 0x97, 0xDE, 0x1C, 0x41, 0x8E, 0xF0};
        const unsigned char arrtest1[] = {0xF0, 0x60, 0x65, 0x00, 0x00, 0x05, 0xF0};
        const unsigned char arrtest2[] = {0xF0, 0x60, 0x65, 0x00, 0x00, 0x06, 0xF0};

        tr9_frame_st *frame = tr9_frame_get_and_need_free(__func__, (unsigned char *)arrtest0, sizeof(arrtest0));
        if (frame != NULL) {
            mcu2rk_hex(frame->cmd, (unsigned char *)arrtest0, sizeof(arrtest0), 5, 5 + frame->data_len - 1);
            free(frame);
        }
        frame = tr9_frame_get_and_need_free(__func__, (unsigned char *)arrtest1, sizeof(arrtest1));
        if (frame != NULL) {
            mcu2rk_hex(frame->cmd, (unsigned char *)arrtest1, sizeof(arrtest1), 5, 5 + frame->data_len - 1);
            free(frame);
        }
        frame = tr9_frame_get_and_need_free(__func__, (unsigned char *)arrtest2, sizeof(arrtest2));
        if (frame != NULL) {
            mcu2rk_hex(frame->cmd, (unsigned char *)arrtest2, sizeof(arrtest2), 5, 5 + frame->data_len - 1);
            free(frame);
        }

        while (1) {
            uart_manage_task();
        }
    }
#elif (0) //测试 //已经被转义过，所以，tr9_frame_get_and_need_free返回值为空
    if (1) {
        const char *str_arr[] = {
            //"F0606500F4A40E11000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000000004AF0",
            "F0606500010501F0",
            "F0171800000FF0",
            "F06065000005F0",
            "F06065000006F0",
            NULL,
        };

        int i_frame = 0;
        while (str_arr[i_frame] != NULL) {
            char *str = (char *)str_arr[i_frame];
            logd("bgbgbgbgbgbgbgbg, \t\t\t%s", str);

            int len_frame = strlen(str);
            if (len_frame < 2 * 7 || ((len_frame % 2) != 0)) {
                loge("err, %d", len_frame);
            }
            len_frame /= 2;

            unsigned char *arr_frame = (unsigned char *)malloc(len_frame);
            if (arr_frame == NULL) {
                loge("err");
            }

            int i;
            for (i = 0; i < len_frame; i++) {
                char *p = (char *)(str + 2 * i);
                sscanf(p, "%2hhx", &arr_frame[i]);
            }

            tr9_frame_st *frame = tr9_frame_get_and_need_free(__func__, (unsigned char *)arr_frame, len_frame);
            if (frame != NULL) {
                mcu2rk_hex(frame->cmd, (unsigned char *)arr_frame, len_frame, 5, 5 + frame->data_len - 1);
                free(frame);
            }

            i_frame++;
            logd("endendendendendend");
        }

        while (1) {
            uart_manage_task();
        }
    }
#endif
}
