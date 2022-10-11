/**
  ******************************************************************************
  * @file    rcd_communicate.h
  * @author  c.w.s
  * @Email
  * @version V1.0.0
  * @date    2013-12-01
  * @brief   行驶记录仪通讯功能
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
	u8  u_read_flag;     //  =0 未执行u盘读取;  =1 等待“确认键按下”; =2 已经按下"确认" 置2
	u8  Data_Direction;  //  =0 平台; =1 U盘; 
	u8  state;			     //成功或失败
	u8 	cmd;			       //命令
  u16 packet;          //发送数据总包数
  u16	idx;			       //当前数据发送包数  
  u16 frame;           //一分包数据包含几个节点数据。
  u16 head;            //当前的节点头指针
  u16 tail;            //当前的节点尾指针
  u16 head_offset;     //头指针偏移指针
  u8  u_send_flag;     // =0, 表示未发送或已经收到发送应答;  =1, 表示在发送中,还未收到应答;
  u32  u8_delay;        // 发送延迟等待时间, 如果延迟时间到还未收到应答,则重新发送
                       //2022-03-31

} rk_6033_task_t;


typedef struct
{
    u16	head; //头指针
    u16	tail;	//尾指针
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
    time_t	start; //开始时间
    time_t	end; //结束时间
    rcd_node_struct p; //数据节点
} rcd_node_option;

typedef struct
{
    rcd_node_struct	cur; //当前节点
    rcd_node_option	ask;	 //数据采集节点
    u8 type; //类型
    u8 from; //数据提取源，UART/NET/USB
    u8 err_cnt;//错误次数
    u8 ack;//等待应答
    u8 id;//消息ID
    u8 ack2;
    u16 start_node;//开始节点标识，用于补报相同的内容
    u16  packnum;//当前包号(块号)
    u16 all_num;	//总包数
    u16 cur_no; //当前包数
    u16 cell_num; //合包个数
    u16 send_jif;	//发送时间，ms
    u16 start_water_no; //采集时终端流水号
    u16 ack_no; //应答平台的流水号
    u16 usbs; //USB no
    //u16 total;
    //u16 send_len;
    u32 total;
    u32 send_len;

    bool retry_en; //使能
    u16 retry_cnt; //总条数
    u16 retry_no; //当前处理条数
    u16 retry_delay_tick;	//延时
} rcd_send_pointor_struct;

typedef struct
{
    u16 len;
    u8  rcdDataBuf[800];
} usb_read_rcd_struct; //usb数据传输控制结构体


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


} usb_read_rcd_data_p; //行驶记录仪指针结构体


typedef struct {   	//C3串口数据
	u8  c3_buf[1024] ;
	u16 c3_len ;

}rec_c3_t;

typedef struct
{
	u8 Data_Direction ;	//数据去向
	u16 packet;			//总包数	
    u16	idx;			//当前包数
} Allot_data_t;

typedef struct
{
	u8  state;			//成功或失败
	u8 	cmd;			//命令
    u16	idx;			//当前包数
} tr9_6033_task_t;


extern u8 Data_Direction ;


extern tr9_6033_task_t  tr9_6033_task	;		//RK应答
extern bool refreshTime ;
extern rec_c3_t c3_data;						//C3的数据
extern Allot_data_t  Allot;						//分包信息
extern rcd_send_pointor_struct	rsp;			//数据传输控制结构体
extern usb_read_rcd_struct usb_rsp;             //usb数据传输控制结构体
extern usb_read_rcd_data_p usb_rcd_pTmp;
extern S_RCD48_SPEED_MANAGE_STRUCT rcd48_m;


extern void rcd_data_send(void);
extern u8 rcd_communicate_parse(u8 from, u8 id, u8 * str, u16 len);
extern void uart_rcd_parse_proc(void);

///////////////////////////////////////
// USB获取记录仪数据相关函数2017-12-15
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

