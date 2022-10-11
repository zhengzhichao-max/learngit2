

#ifndef __GBT_19056_2012_BIAO_C_1_IC_CARD_H__
#define __GBT_19056_2012_BIAO_C_1_IC_CARD_H__

extern const DRIVER_CONFIG_STRUCT_RANGE driver_cfg_range;

int isName_ic_card(char *str);
int isShenFenZheng_ic_card(const char *str);
int isYouXiaoQi_ic_card(const char *str, int *year, int *month,
                        int *day);
int isBianhao_ic_card(const char *str);
int isJiaShiZheng_ic_card(const char *str);

#endif /*__GBT_19056_2012_BIAO_C_1_IC_CARD_H__*/
