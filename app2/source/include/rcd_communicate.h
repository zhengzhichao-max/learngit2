/**
  ******************************************************************************
  * @file    rcd_communicate.h
  * @author  c.w.s
  * @Email
  * @version V1.0.0
  * @date    2013-12-01
  * @brief   ��ʻ��¼��ͨѶ����
  ******************************************************************************
  * @attention

  ******************************************************************************
*/
#ifndef	__RCD_COMMUNICATE_H
#define	__RCD_COMMUNICATE_H

#define RCD_C_OK				0x00
#define RCD_C_GET_ERR			0x01
#define RCD_C_SET_ERR			0x02

#define D_BUF_LEN				200

#define FRAME_DATA_SIZE		800

//2022-03-17 add by hj
typedef struct
{
	u8  u_read_flag;     //  =0 δִ��u�̶�ȡ;  =1 �ȴ���ȷ�ϼ����¡�; =2 �Ѿ�����"ȷ��" ��2
	u8  Data_Direction;  //  =0 ƽ̨; =1 U��; 
	u8  state;			     //�ɹ���ʧ��
	u8 	cmd;			       //����
  u16 packet;          //���������ܰ���
  u16	idx;			       //��ǰ���ݷ��Ͱ���  
  u16 frame;           //һ�ְ����ݰ��������ڵ����ݡ�
  u16 head;            //��ǰ�Ľڵ�ͷָ��
  u16 tail;            //��ǰ�Ľڵ�βָ��
  u16 head_offset;     //ͷָ��ƫ��ָ��
  u8  u_send_flag;     // =0, ��ʾδ���ͻ��Ѿ��յ�����Ӧ��;  =1, ��ʾ�ڷ�����,��δ�յ�Ӧ��;
  u32  u8_delay;        // �����ӳٵȴ�ʱ��, ����ӳ�ʱ�䵽��δ�յ�Ӧ��,�����·���
                       //2022-03-31

} rk_6033_task_t;


typedef struct
{
    u16	head; //ͷָ��
    u16	tail;	//βָ��
} rcd_node_struct;

static enum
{
   Cmd_0x00H = 0 ,
   Cmd_0x01H,
   Cmd_0x02H,
   Cmd_0x03H,
   Cmd_0x04H,
   Cmd_0x05H,
   Cmd_0x06H,
   Cmd_0x07H,
   Cmd_0x08H,
   Cmd_0x09H,
   Cmd_0x10H = 16,
   Cmd_0x11H,
   Cmd_0x12H,
   Cmd_0x13H,
   Cmd_0x14H,
   Cmd_0x15H,
}Tachographs_t;



typedef struct
{
    time_t	start; //��ʼʱ��
    time_t	end; //����ʱ��
    rcd_node_struct p; //���ݽڵ�
} rcd_node_option;

typedef struct
{
    rcd_node_struct	cur; //��ǰ�ڵ�
    rcd_node_option	ask;	 //���ݲɼ��ڵ�
    u8 type; //����
    u8 from; //������ȡԴ��UART/NET/USB
    u8 err_cnt;//�������
    u8 ack;//�ȴ�Ӧ��
    u8 id;//��ϢID
    u8 ack2;
    u16 start_node;//��ʼ�ڵ��ʶ�����ڲ�����ͬ������
    u16  packnum;//��ǰ����(���)
    u16 all_num;	//�ܰ���
    u16 cur_no; //��ǰ����
    u16 cell_num; //�ϰ�����
    u16 send_jif;	//����ʱ�䣬ms
    u16 start_water_no; //�ɼ�ʱ�ն���ˮ��
    u16 ack_no; //Ӧ��ƽ̨����ˮ��
    u16 usbs; //USB no
    //u16 total;
    //u16 send_len;
    u32 total;
    u32 send_len;

    bool retry_en; //ʹ��
    u16 retry_cnt; //������
    u16 retry_no; //��ǰ��������
    u16 retry_delay_tick;	//��ʱ
} rcd_send_pointor_struct;

typedef struct
{
    u16 len;
    u8  rcdDataBuf[800];
} usb_read_rcd_struct; //usb���ݴ�����ƽṹ��


typedef struct
{
	bool rcd360_step_flg;
	bool rcd48_step_flg;
	bool rcd_accident_flag;
	bool OT_flag;
	bool driver_log_flag;
	bool  power_flag;
	bool  para_change_flag;

    u16   rcd360_p_head;
    u16   rcd360_p_tail;

    u16   rcd48_p_head;
    u16   rcd48_p_tail;
    
    u16   rcd_accident_p_head;
    u16   rcd_accident_p_tail;
    
    u16   OT_pHead;
    u16   OT_pTail;
    
    u16 driver_log_pHead;
    u16 driver_log_pTail;
    
    u16  power_log_pHead;
    u16  power_log_pTail;
   
    u16  para_change_pHead;
    u16  para_change_pTail;

    u16 speed_log_pHead;
    u16 speed_log_pTail;


} usb_read_rcd_data_p; //��ʻ��¼��ָ��ṹ��


typedef struct {   	//C3��������
	u8  c3_buf[1024] ;
	u16 c3_len ;

}rec_c3_t;

typedef struct
{
	u8 Data_Direction ;	//����ȥ��
	u16 packet;			//�ܰ���	
    u16	idx;			//��ǰ����
} Allot_data_t;

typedef struct
{
	u8  state;			//�ɹ���ʧ��
	u8 	cmd;			//����
    u16	idx;			//��ǰ����
} tr9_6033_task_t;


extern u8 Data_Direction ;


extern tr9_6033_task_t  tr9_6033_task	;		//RKӦ��
extern bool refreshTime ;
extern rec_c3_t c3_data;						//C3������
extern Allot_data_t  Allot;						//�ְ���Ϣ
extern rcd_send_pointor_struct	rsp;			//���ݴ�����ƽṹ��
extern usb_read_rcd_struct usb_rsp;             //usb���ݴ�����ƽṹ��
extern usb_read_rcd_data_p usb_rcd_pTmp;
extern S_RCD48_SPEED_MANAGE_STRUCT rcd48_m;


extern void rcd_data_send(void);
extern u8 rcd_communicate_parse(u8 from, u8 id, u8 * str, u16 len);
extern void uart_rcd_parse_proc(void);

///////////////////////////////////////
// USB��ȡ��¼��������غ���2017-12-15
///////////////////////////////////////
extern void USB_get_rcd_ver(void);
extern void USB_get_driver(void);
extern void USB_get_real_time(void);
extern void USB_get_mileage(void);
extern void USB_get_pulse(void);
extern void USB_get_vehicle_ifno(void);
extern void USB_get_vehicle_status_info(void);
extern void USB_get_rcd_unique_ID(void);
extern bool USB_get_rcd_48speed(void);
extern bool USB_get_rcd_360loction(void);
extern bool USB_get_driver_log(void);
extern bool USB_get_para_rcd(void);
extern void USB_get_speed_log(void);
extern bool USB_get_power_log(void);
extern bool USB_get_accident(void);
extern bool USB_get_driver_OT(void);

//extern void tr9_Driving_Record_update_Rk(u8 command,u8 dion);
extern void Rk_Driving_Record_Up(u8 cmd);

#endif	/* __RCD_COMMUNICATE_H */

