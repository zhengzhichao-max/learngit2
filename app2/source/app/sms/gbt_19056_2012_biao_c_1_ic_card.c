

#include "include_all.h"

const DRIVER_CONFIG_STRUCT_RANGE driver_cfg_range = {
    .name = {1, DRIVER_CONFIG_STRUCT_NAME_MAX},         //包括
    .bian_hao = {1, DRIVER_CONFIG_STRUCT_BIAN_HAO_MAX}, //包括
};

/*
0,    是
-1, 不是
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
    int province;    // = -1; //1-2:省（直辖市、自治区）
    int city;        // = -1; //3-4:市（自治州）
    int distinguish; // = -1; //5-6:区（县、自治县、县级市）

    int year;  // = -1; //7-8:年
    int month; // = -1; //9-10:月
    int day;   // = -1; //11-12:日
    int num;   // = -1; // 13-15:为顺序号，其中15位男为单数，女为双数
    int sex;
} sfz15_st;
const char pattern_sfz15[] = "%2d%2d%2d%2d%2d%2d%3d";
#endif

#if (0)
typedef struct {
    int province;    //  = -1;  //1-2:省（直辖市、自治区）
    int city;        // = -1;        //3-4:市（自治州）
    int distinguish; // = -1; //5-6:区（县、自治县、县级市）

    int year;  // = -1; //7-10:年
    int month; // = -1;       //11-12:月
    int day;   // = -1;         //13-14:日

    int police;      // = -1; //15-16:派出所代码
    int sex;         // = -1;    //17:性别：奇数表示男性，偶数表示女性
    char str_sum[2]; //  = -{0};//18:校检码
    int sum;         // = -1;    //18:校检码
} sfz18_st;
const char pattern_sfz18[] = "%2d%2d%2d%4d%2d%2d%2d%1d%1s";
#endif

/*
0,    是
-1, 不是
*/
/*
15位身份证号码各位的含义:

1、1-2位省、自治区、直辖市代码；

2、3-4位地级市、盟、自治州代码；

3、5-6位县、县级市、区代码；

4、7-12位出生年月日,比如670401代表1967年4月1日，与18位的第一个区别；

5、13-15位为顺序号，其中15位男为单数，女为双数；

举例介绍：

130503 670401 001的含义;
13为河北，05为邢台，03为桥西区，出生日期为1967年4月1日，顺序号为001。
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
0,    是
-1, 不是
*/
/*
18位的第二代身份证号码是特征组合码，由十七位数字本体码和一位校验码组成。排列顺序从左至右依次为：六位数字地址码、八位数字出生日期码、三位数字顺序码和一位数字校验码。

地址码（前六位数）：表示身份证编码对象常住户口所在省份、地市，以及县（市、旗、区）的行政区划代码，按照GB/T2260的规定执行。其中：前1、2位数字表示所在省份的代码；
第3、4位数字表示所在地市的代码；
第5、6位数字表示：所在县（市、旗、区）的代码。

出生日期码（第七位至十四位）：表示身份证编码对象出生的年、月、日，按GB/T7408的规定执行，年、月、日代码之间不用分隔符。

顺序码（第十五位至十七位）：表示在同一地址码所标识的区域范围内，对同年、同月、同日出生的人编定的顺序号，顺序码的奇数分配给男性，偶数分配给女性。

校验码（第十八位数）：作为尾号的校验码，是由身份证号码编制单位按统一的公式计算出来的，如果某人的尾号是0－9，都不会出现Ⅹ，但如果尾号是10，那么就得用Ⅹ来代替。因为如果用10做尾号，那么此人的身份证就变成了19位。
*/

/*
一、身份证号码作为独一无二代表公民信息的号码，它的组成是由一下的数字表示的：
（1）第1、2位数字表示：所在省（直辖市、自治区）的代码；
（2）第3、4位数字表示：所在地级市（自治州）的代码；
（3）第5、6位数字表示：所在区（县、自治县、县级市）的代码；
（4）第7—14位数字表示：出生年、月、日；
（5）第15、16位数字表示：所在地的派出所的代码；
（6）第17位数字表示性别：奇数表示男性，偶数表示女性；
（7）第18位数字是校检码：是根据《中华人民共和国国家标准GB
11643-1999》中有关公民身份号码的规定，根据精密的计算公式计算出来的。

二、校验码的计算方法：
1、将前面的身份证号码17位数分别乘以不同的系数。从第一位到第十七位的系数分别为:7、9、10、5、8、4、2、1、6、3、7、9、10、5、8、4、2
; 2、将这17位数字和系数相乘的结果相加; 3、用加出来和除以11，看余数是多少;
4、余数只可能有0、1、2、3、4、5、6、7、8、9、10这11个数字。其分别对应的最后一位身份证的号码为1、0、X、9、8、7、6、5、4、3、2，其中的X是罗马数字10;
5、通过上面得知如果余数是2，就会在身份证的第18位数字上出现罗马数字的Ⅹ；如果余数是10，身份证的最后一位号码就是2
————————————————
版权声明：本文为CSDN博主「MrBlackWhite」的原创文章，遵循CC 4.0
BY-SA版权协议，转载请附上原文出处链接及本声明。
原文链接：https://blog.csdn.net/u011665991/article/details/102727485
*/
/*
string[] aCity = new
string[]{null,null,null,null,null,null,null,null,null,null,null,"北京","天津","河北","山西","内蒙古",null,null,null,null,null,"辽宁","吉林","黑龙江",null,null,null,null,null,null,null,"上海","江苏","浙江","安微","福建","江西","山东",null,null,null,"河南","湖北","湖南","广东","广西","海南",null,null,null,"重庆","四川","贵州","云南","西藏",null,null,null,null,null,null,"陕西","甘肃","青海","宁夏","新疆",null,null,null,null,null,"台湾",null,null,null,null,null,null,null,null,null,"香港","澳门",null,null,null,null,null,null,null,null,"国外"};
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

    // 1.将身份证号码前面的17位数分别乘以不同的系数。
    const int coefficientArr[] = {7, 9, 10, 5, 8, 4, 2, 1, 6,
                                  3, 7, 9, 10, 5, 8, 4, 2};
    const char lastChar[] = {'1', '0', 'X', '9', '8', '7',
                             '6', '5', '4', '3', '2'};
    int i = 0;
    int sum = 0;
    for (; i < 17; i++) {
        // Character.digit
        // 在指定的基数返回字符ch的数值。如果基数是不在范围内MIN_RADIX≤基数≤MAX_RADIX或如果该值的通道是不是一个有效的数字在指定的基数-1，则返回。
        // ch - the character to be converted(要转换的字符)
        // ch - int类型，是字符的ASCII码，数字的ASCII码是48-57
        // radix - the radix(基数) ----也就是进制数
        sum += ((str[i] - '0') * coefficientArr[i]);
    }

    sum %= 11;

    if (str[17] != lastChar[sum]) {
        return -1;
    }

    return 0;
}

/*
判断是不是驾驶证
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
判断有效期
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
判断：司机编号/工号
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
大车驾驶证，不是身份证？
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
判断是不是驾驶证
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
