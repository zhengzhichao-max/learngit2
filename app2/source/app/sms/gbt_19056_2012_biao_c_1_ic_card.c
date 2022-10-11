

#include "include_all.h"

const DRIVER_CONFIG_STRUCT_RANGE driver_cfg_range = {
    .name = {1, DRIVER_CONFIG_STRUCT_NAME_MAX},         //����
    .bian_hao = {1, DRIVER_CONFIG_STRUCT_BIAN_HAO_MAX}, //����
};

/*
0,    ��
-1, ����
*/
int isName_ic_card(char *str) {
    if (str == NULL) {
        return -1;
    }

    // logd("<%s>", str);
    int len = strlen(str);

    if (!(len >= driver_cfg_range.name.min && len <= driver_cfg_range.name.max)) {
        loge("(!(len >= %d && len <= %d)), %d", driver_cfg_range.name.min,
             driver_cfg_range.name.max, len);
        return -1;
    }

    return 0;
}

#if (1)
typedef struct {
    int province;    // = -1; //1-2:ʡ��ֱϽ�С���������
    int city;        // = -1; //3-4:�У������ݣ�
    int distinguish; // = -1; //5-6:�����ء������ء��ؼ��У�

    int year;  // = -1; //7-8:��
    int month; // = -1; //9-10:��
    int day;   // = -1; //11-12:��
    int num;   // = -1; // 13-15:Ϊ˳��ţ�����15λ��Ϊ������ŮΪ˫��
    int sex;
} sfz15_st;
const char pattern_sfz15[] = "%2d%2d%2d%2d%2d%2d%3d";
#endif

#if (0)
typedef struct {
    int province;    //  = -1;  //1-2:ʡ��ֱϽ�С���������
    int city;        // = -1;        //3-4:�У������ݣ�
    int distinguish; // = -1; //5-6:�����ء������ء��ؼ��У�

    int year;  // = -1; //7-10:��
    int month; // = -1;       //11-12:��
    int day;   // = -1;         //13-14:��

    int police;      // = -1; //15-16:�ɳ�������
    int sex;         // = -1;    //17:�Ա�������ʾ���ԣ�ż����ʾŮ��
    char str_sum[2]; //  = -{0};//18:У����
    int sum;         // = -1;    //18:У����
} sfz18_st;
const char pattern_sfz18[] = "%2d%2d%2d%4d%2d%2d%2d%1d%1s";
#endif

/*
0,    ��
-1, ����
*/
/*
15λ���֤�����λ�ĺ���:

1��1-2λʡ����������ֱϽ�д��룻

2��3-4λ�ؼ��С��ˡ������ݴ��룻

3��5-6λ�ء��ؼ��С������룻

4��7-12λ����������,����670401����1967��4��1�գ���18λ�ĵ�һ������

5��13-15λΪ˳��ţ�����15λ��Ϊ������ŮΪ˫����

�������ܣ�

130503 670401 001�ĺ���;
13Ϊ�ӱ���05Ϊ��̨��03Ϊ����������������Ϊ1967��4��1�գ�˳���Ϊ001��
*/
/*
char str_sfz15_1[]="  130503670401001  ";
unsigned char ret_sfz15_1 = isShenFenZheng15((const char *)str_sfz15_1);
char str_sfz15_2[]="130503670401001";
unsigned char ret_sfz15_2 = isShenFenZheng15((const char *)str_sfz15_2);
char str_sfz15_3[]="130503670401006";
unsigned char ret_sfz15_3 = isShenFenZheng15((const char *)str_sfz15_3);
*/
static int isShenFenZheng15(const char *str) {
    if (str == NULL) {
        return -1;
    }

    int len = strlen(str);

    if (len != 15) {
        loge("len != 15, %d", len);
#if (MODULE_DEBUG_ic_card)
        putHexs_hex((unsigned char*)str, len);
#endif
        return -1;
    }

    // logd("sfz15<%s>", str);

#if (1)
    sfz15_st sfz15 = {.province = -1,
                      .city = -1,
                      .distinguish = -1,
                      .year = -1,
                      .month = -1,
                      .day = -1,
                      .num = -1,
                      .sex = -1};

    int ret = sscanf(str, pattern_sfz15, &sfz15.province, &sfz15.city,
                     &sfz15.distinguish, &sfz15.year, &sfz15.month, &sfz15.day,
                     &sfz15.num);

    if (ret != 7) {
        loge("ret != 7, %d", ret);
        return -1;
    }

    sfz15.sex = sfz15.num % 10;

    logd("%02d,%02d,%02d,19%02d,%02d,%02d,%03d,%d", sfz15.province, sfz15.city,
         sfz15.distinguish, sfz15.year, sfz15.month, sfz15.day, sfz15.num,
         sfz15.sex);
#endif

    return 0;
}

/*
0,    ��
-1, ����
*/
/*
18λ�ĵڶ������֤��������������룬��ʮ��λ���ֱ������һλУ������ɡ�����˳�������������Ϊ����λ���ֵ�ַ�롢��λ���ֳ��������롢��λ����˳�����һλ����У���롣

��ַ�루ǰ��λ��������ʾ���֤�������ס��������ʡ�ݡ����У��Լ��أ��С��졢�����������������룬����GB/T2260�Ĺ涨ִ�С����У�ǰ1��2λ���ֱ�ʾ����ʡ�ݵĴ��룻
��3��4λ���ֱ�ʾ���ڵ��еĴ��룻
��5��6λ���ֱ�ʾ�������أ��С��졢�����Ĵ��롣

���������루����λ��ʮ��λ������ʾ���֤�������������ꡢ�¡��գ���GB/T7408�Ĺ涨ִ�У��ꡢ�¡��մ���֮�䲻�÷ָ�����

˳���루��ʮ��λ��ʮ��λ������ʾ��ͬһ��ַ������ʶ������Χ�ڣ���ͬ�ꡢͬ�¡�ͬ�ճ������˱ඨ��˳��ţ�˳�����������������ԣ�ż�������Ů�ԡ�

У���루��ʮ��λ��������Ϊβ�ŵ�У���룬�������֤������Ƶ�λ��ͳһ�Ĺ�ʽ��������ģ����ĳ�˵�β����0��9����������֢��������β����10����ô�͵��â������档��Ϊ�����10��β�ţ���ô���˵����֤�ͱ����19λ��
*/

/*
һ�����֤������Ϊ��һ�޶���������Ϣ�ĺ��룬�����������һ�µ����ֱ�ʾ�ģ�
��1����1��2λ���ֱ�ʾ������ʡ��ֱϽ�С����������Ĵ��룻
��2����3��4λ���ֱ�ʾ�����ڵؼ��У������ݣ��Ĵ��룻
��3����5��6λ���ֱ�ʾ�����������ء������ء��ؼ��У��Ĵ��룻
��4����7��14λ���ֱ�ʾ�������ꡢ�¡��գ�
��5����15��16λ���ֱ�ʾ�����ڵص��ɳ����Ĵ��룻
��6����17λ���ֱ�ʾ�Ա�������ʾ���ԣ�ż����ʾŮ�ԣ�
��7����18λ������У���룺�Ǹ��ݡ��л����񹲺͹����ұ�׼GB
11643-1999�����йع�����ݺ���Ĺ涨�����ݾ��ܵļ��㹫ʽ��������ġ�

����У����ļ��㷽����
1����ǰ������֤����17λ���ֱ���Բ�ͬ��ϵ�����ӵ�һλ����ʮ��λ��ϵ���ֱ�Ϊ:7��9��10��5��8��4��2��1��6��3��7��9��10��5��8��4��2
; 2������17λ���ֺ�ϵ����˵Ľ�����; 3���üӳ����ͳ���11���������Ƕ���;
4������ֻ������0��1��2��3��4��5��6��7��8��9��10��11�����֡���ֱ��Ӧ�����һλ���֤�ĺ���Ϊ1��0��X��9��8��7��6��5��4��3��2�����е�X����������10;
5��ͨ�������֪���������2���ͻ������֤�ĵ�18λ�����ϳ����������ֵĢ������������10�����֤�����һλ�������2
��������������������������������
��Ȩ����������ΪCSDN������MrBlackWhite����ԭ�����£���ѭCC 4.0
BY-SA��ȨЭ�飬ת���븽��ԭ�ĳ������Ӽ���������
ԭ�����ӣ�https://blog.csdn.net/u011665991/article/details/102727485
*/
/*
string[] aCity = new
string[]{null,null,null,null,null,null,null,null,null,null,null,"����","���","�ӱ�","ɽ��","���ɹ�",null,null,null,null,null,"����","����","������",null,null,null,null,null,null,null,"�Ϻ�","����","�㽭","��΢","����","����","ɽ��",null,null,null,"����","����","����","�㶫","����","����",null,null,null,"����","�Ĵ�","����","����","����",null,null,null,null,null,null,"����","����","�ຣ","����","�½�",null,null,null,null,null,"̨��",null,null,null,null,null,null,null,null,null,"���","����",null,null,null,null,null,null,null,null,"����"};
*/
static int isShenFenZheng18(const char *str) {
    if (str == NULL) {
        return -1;
    }

    int len = strlen(str);

    if (len != 18) {
        loge("(len != 18), %d", len);
#if (MODULE_DEBUG_ic_card)
        putHexs_hex((unsigned char *)str, len);
#endif
        return -1;
    }

    // 1.�����֤����ǰ���17λ���ֱ���Բ�ͬ��ϵ����
    const int coefficientArr[] = {7, 9, 10, 5, 8, 4, 2, 1, 6,
                                  3, 7, 9, 10, 5, 8, 4, 2};
    const char lastChar[] = {'1', '0', 'X', '9', '8', '7',
                             '6', '5', '4', '3', '2'};
    int i = 0;
    int sum = 0;
    for (; i < 17; i++) {
        // Character.digit
        // ��ָ���Ļ��������ַ�ch����ֵ����������ǲ��ڷ�Χ��MIN_RADIX�ܻ�����MAX_RADIX�������ֵ��ͨ���ǲ���һ����Ч��������ָ���Ļ���-1���򷵻ء�
        // ch - the character to be converted(Ҫת�����ַ�)
        // ch - int���ͣ����ַ���ASCII�룬���ֵ�ASCII����48-57
        // radix - the radix(����) ----Ҳ���ǽ�����
        sum += ((str[i] - '0') * coefficientArr[i]);
    }

    sum %= 11;

    if (str[17] != lastChar[sum]) {
        return -1;
    }

    return 0;
}

/*
�ж��ǲ��Ǽ�ʻ֤
*/
int isShenFenZheng_ic_card(const char *str) {
    int ret = isShenFenZheng18(str);
    if (ret == 0) {
        return 0;
    }

    ret = isShenFenZheng15(str);
    if (ret == 0) {
        return 0;
    }

    return -1;
}

/*
�ж���Ч��
*/
int isYouXiaoQi_ic_card(const char *str, int *year, int *month,
                        int *day) {
    if (str == NULL) {
        return -1;
    }

    int len = strlen(str);

    if (len != 6) {
        loge("(len != 6), %d", len);
        return -1;
    }

    int ret = sscanf(str, "%2d%2d%2d", year, month, day);

    if (ret != 3) {
        loge("ret != 3, %d", ret);
        return -1;
    }

    if (!(*month >= 1 && *month <= 12)) {
        loge("month err, %d", *month);
        return -1;
    }

    if (!(*day >= 1 && *day <= 31)) {
        loge("day err, %d", *day);
        return -1;
    }

    return 0;
}

/*
�жϣ�˾�����/����
*/
int isBianhao_ic_card(const char *str) {
    if (str == NULL) {
        return -1;
    }

    int len = strlen(str);

    if (!(len >= driver_cfg_range.bian_hao.min && len <= driver_cfg_range.bian_hao.max)) {
        loge("(!(len >= %d && len <= %d)), %d", driver_cfg_range.bian_hao.min,
             driver_cfg_range.bian_hao.max, len);
        return -1;
    }

    int i = 0;
    int ret = 0;
    for (; i < len; i++) {
        int diff = (int)(str[i] - '0');

        if (!(diff >= 0 && diff <= 9)) {
            loge("BH err");
            ret = -1;
            break;
        }
    }

    return ret;
}

/*
�󳵼�ʻ֤���������֤��
*/
static int isJiaShiZheng_DaChe(const char *str) {
    if (str == NULL) {
        return -1;
    }

    int len = strlen(str);

    if (!(len >= 12 && len <= 18)) {
        loge("len err, %d", len);
#if (MODULE_DEBUG_ic_card)
        putHexs_hex((unsigned char *)str, len);
#endif
        return -1;
    }

    return 0;
}

/*
�ж��ǲ��Ǽ�ʻ֤
*/
int isJiaShiZheng_ic_card(const char *str) {
    int ret = isShenFenZheng18(str);
    if (ret == 0) {
#if (MODULE_DEBUG_ic_card)
        logf("is jsz");
#endif
        return 0;
    }

    ret = isShenFenZheng15(str);
    if (ret == 0) {
#if (MODULE_DEBUG_ic_card)
        logf("is jsz");
#endif
        return 0;
    }

    ret = isJiaShiZheng_DaChe(str);
    if (ret == 0) {
#if (MODULE_DEBUG_ic_card)
        logf("is jsz");
#endif
        return 0;
    }

    return -1;
}
