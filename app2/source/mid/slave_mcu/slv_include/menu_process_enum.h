
#ifndef __MENU_PRCESS_ENUM_H__
#define __MENU_PRCESS_ENUM_H__

#define menu_count(a) (a - 1)
#define menu_line(a) (a - 1)

//�ٶ�ģʽ
//220828�ٶ�����://ȫ�ֱ�����speed_mode//ö����ʾ�ַ���
typedef enum {
    enum_speed_mode_null,         //��Чֵ
    enum_speed_mode_pusle,        //�����ٶ�//pulse
    enum_speed_mode_can,          //can//wheel_speed
    enum_speed_mode_gps,          //GPS//mix.speed
    enum_speed_mode_set_password, //��������
    enum_speed_mode_cnt,          //�ȳ��ȶ���1
} enum_speed_mode_typedef;

//�ն˿���//terminal_ctrl
typedef enum {
    enum_terminal_ctrl__null,        //��Чֵ
    enum_terminal_ctrl_restart,      //�ն�����
    enum_terminal_ctrl_register,     //�ն�ע��
    enum_terminal_ctrl_authenticate, //�ն˼�Ȩ
    enum_terminal_ctrl_logout,       //�ն�ע��
    enum_terminal_ctrl_lock,         //�ն�����
    enum_terminal_ctrl_unlock,       //�ն˽���
    enum_terminal_ctrl_foster_care,  //ϵͳ����
    enum_terminal_ctrl_cnt,          //�ȳ��ȶ���1
} enum_terminal_ctl_typedef;

#endif //__MENU_PRCESS_ENUM_H__
