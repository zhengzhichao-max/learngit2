#include "include_all.h"

#if (P_CAN == _CAN_BASE)

CanRxMsg RxMessage;  //接收CAN1的数据
CanRxMsg RxMessage2; //接收CAN2的数据

u8 CAN_msg_num[MAX_MAIL_NUM];  // 发送邮箱标记 CAN1
u8 CAN_msg_num2[MAX_MAIL_NUM]; // 发送邮箱标记 CAN2

u8 CAN_buf[CAN_DATA_SIZE] = {0};  //CAN1数据存放区
u8 CAN_buf2[CAN_DATA_SIZE] = {0}; //CAN2数据存放区

u16 can_limit_rabio = 0;
//static u8 cun =0;
CAN_param_t Can_param;
CAN_PARAM_GET_T Can_param_get;
CAN_ID_CONFIG_T Can_id_config;  //组包是使用
CAN_ID_CONFIG_T Can_id_config2; //组包是使用

Set_Can_t tr9_Set_Can[28];       //单个CAN的设置
SOFTEWARE_LOCK_T Wofteware_lock; //软件锁

static CAN_ID_FILTRATION_T Can_Id[14]; //最多可以设置28个CAN_ID

CAN_SEND_DATA cansendbuf;  //保存CAN1的数据
CAN_SEND_DATA cansendbuf2; //保存CAN2的数据

////可以工作在列表模式  或  掩码模式

#if 0
void InitCan(void)
{
	CAN_InitTypeDef			CAN_InitStructure;
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;

	//u32 Id0 = 0x18fef100;    //EEC1，发动机控制器
	//u32 Id1 = 0x18fef200;	 //巡航控制车速，车速信号
	u32 Id0 = 0x18fef100;    //EEC1，发动机控制器
	u32 Id1 = 0x18fef200;	 //巡航控制车速，车速信号


    //互联型有两个CAN控制器
	CAN_DeInit(CAN1);                           //将CAN1所有的寄存器重设为缺省值                           
	CAN_StructInit(&CAN_InitStructure);         //初始化 CAN_InitStructure 结构体

	CAN_InitStructure.CAN_TTCM=DISABLE;         //不使能时间触发通讯模式
	CAN_InitStructure.CAN_ABOM=ENABLE;          //使能自动离线管理
	CAN_InitStructure.CAN_AWUM=DISABLE;         //不使能自动唤醒模式
	CAN_InitStructure.CAN_NART=DISABLE;         //不使能非自动重传输模式
	CAN_InitStructure.CAN_RFLM=DISABLE;         //不使能FIFO锁定模式,新的报文直接覆盖旧的报文
	CAN_InitStructure.CAN_TXFP=DISABLE;         //不使能发送FIFO优先级，优先级由报文标识符决定 
	CAN_InitStructure.CAN_Mode=CAN_Mode_Normal; //正常模式 
    //设置波特率
	CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;      //重新同步跳跃宽度 1 个时间单位
	CAN_InitStructure.CAN_BS1=CAN_BS1_5tq;	    //时间段 1 为 5 个时间单位
	CAN_InitStructure.CAN_BS2=CAN_BS2_6tq;	    //时间段 2 为 6 个时间单位 250K
	CAN_InitStructure.CAN_Prescaler=6; 	        //设定了一个时间单位的长度，它的范围是 1 到 1024。	
	                       						// CANbps=APB1总线频率18000000/6/(6+5+1))=250k bps  
	CAN_Init(CAN1,&CAN_InitStructure);

#if 1
	CAN_FilterInitStructure.CAN_FilterNumber = 0;                       //指定了待初始化的过滤器，它的范围是 1 到 13  
	CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdList;   //标识符列表模式
	CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;   // 32 位过滤器

	//对扩展数据帧进行过滤:(只接收扩展数据帧) 扩展ID为29bit
	CAN_FilterInitStructure.CAN_FilterIdHigh = 		(((u32)Id0<<3)&0xFFFF0000)>>16;
	CAN_FilterInitStructure.CAN_FilterIdLow  = 		(((u32)Id0<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 	(((u32)Id1<<3)&0xFFFF0000)>>16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  = 	(((u32)Id1<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

	CAN_FilterInit(&CAN_FilterInitStructure);

#endif
	
	// 使能FIFO0消息挂号中断、FIFO0满中断、FIFO0溢出中断
	CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);  
	//不使能发送邮箱空中断
   // CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);       
    //使能错误警告中断、错误被动中断、离线中断、上次错误号中断、错误中断、唤醒中断、睡眠标志位中断
   // CAN_ITConfig(CAN1, CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC 
    //                 | CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);    // ERR中断
    // CAN缓存初始化
    //memset(CAN_msg_num,0,MAX_MAIL_NUM);
    _memset(CAN_msg_num,0,MAX_MAIL_NUM);
	_memset(tr9_Set_Can, 0x00, sizeof(Set_Can_t)*28);
	_memset(CAN_ID_FILTRATION_T, 0x00, sizeof(CAN_ID_FILTRATION_T)*28);
}

#endif
const u32 extCAN_CONTRAL_SPEED = {0x0C000003};

/***************************************************************************************
* 函数名称：InitCan
* 函数功能：初始化CAN1
* 函数参数：无1
* 时     间:	2022.2.25
* 备     注：CAN_FilterMode_IdMask 这个是掩码模式				//在这个模式下全部ID和数据都能进来
*			CAN_FilterMode_IdList 这个是列表模式			//在这个模式下只能收到列表中的ID
*			
*			CAN_FilterInitStructure.CAN_FilterIdHigh =		0 ;
*			CAN_FilterInitStructure.CAN_FilterIdLow  =		0 ;	
*			CAN_FilterInitStructure.CAN_FilterMaskIdHigh =	0 ;
*			CAN_FilterInitStructure.CAN_FilterMaskIdLow  =	0 ;	
*
***************************************************************************************/
void InitCan(void) {
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    //u32 Id0 = 0x18fef100;    //EEC1，发动机控制器
    //u32 Id1 = 0x18fef200;	 //巡航控制车速，车速信号
    //u32 Id0 = 0x18fef100;    //EEC1，发动机控制器
    //u32 Id1 = 0x18fef200;	 //巡航控制车速，车速信号

    //互联型有两个CAN控制器
    CAN_DeInit(CAN1);                   //将CAN1所有的寄存器重设为缺省值
    CAN_StructInit(&CAN_InitStructure); //初始化 CAN_InitStructure 结构体

    CAN_InitStructure.CAN_TTCM = DISABLE;         //不使能时间触发通讯模式
    CAN_InitStructure.CAN_ABOM = ENABLE;          //使能自动离线管理
    CAN_InitStructure.CAN_AWUM = DISABLE;         //不使能自动唤醒模式
    CAN_InitStructure.CAN_NART = DISABLE;         //不使能非自动重传输模式
    CAN_InitStructure.CAN_RFLM = DISABLE;         //不使能FIFO锁定模式,新的报文直接覆盖旧的报文
    CAN_InitStructure.CAN_TXFP = DISABLE;         //不使能发送FIFO优先级，优先级由报文标识符决定
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; //正常模式
        //设置波特率
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq; //重新同步跳跃宽度 1 个时间单位
    CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq; //时间段 1 为 5 个时间单位
    CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq; //时间段 2 为 6 个时间单位 250K
    CAN_InitStructure.CAN_Prescaler = 6;     //设定了一个时间单位的长度，它的范围是 1 到 1024。
                                             // CANbps=APB1总线频率18000000/6/(6+5+1))=250k bps
    CAN_Init(CAN1, &CAN_InitStructure);

#if 1
    CAN_FilterInitStructure.CAN_FilterNumber = 1;                    //指定了待初始化的过滤器，它的范围是 0 到 13
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  //无掩码模式
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // 32 位过滤器

    //对扩展数据帧进行过滤:(只接收扩展数据帧) 扩展ID为29bit
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&CAN_FilterInitStructure);

#endif

    // 使能FIFO0消息挂号中断、FIFO0满中断、FIFO0溢出中断
    CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);
    //不使能发送邮箱空中断
    // CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
    //使能错误警告中断、错误被动中断、离线中断、上次错误号中断、错误中断、唤醒中断、睡眠标志位中断
    // CAN_ITConfig(CAN1, CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC
    //                 | CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);    // ERR中断
    // CAN缓存初始化
    //memset(CAN_msg_num,0,MAX_MAIL_NUM);
    _memset(CAN_msg_num, 0, MAX_MAIL_NUM);
    _memset((u8 *)tr9_Set_Can, 0x00, sizeof(Set_Can_t) * 28);
    _memset((u8 *)Can_Id, 0x00, sizeof(CAN_ID_FILTRATION_T) * 14);
}

void InitCan2(void) {
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    //u32 Id0 = 0x18fef100;    //EEC1，发动机控制器
    //u32 Id1 = 0x18fef200;	 //巡航控制车速，车速信号
    //u32 Id0 = 0x18fef100;    //EEC1，发动机控制器
    //u32 Id1 = 0x18fef200;	 //巡航控制车速，车速信号

    //互联型有两个CAN控制器
    CAN_DeInit(CAN2);                   //将CAN1所有的寄存器重设为缺省值
    CAN_StructInit(&CAN_InitStructure); //初始化 CAN_InitStructure 结构体

    CAN_InitStructure.CAN_TTCM = DISABLE;         //不使能时间触发通讯模式
    CAN_InitStructure.CAN_ABOM = ENABLE;          //使能自动离线管理
    CAN_InitStructure.CAN_AWUM = DISABLE;         //不使能自动唤醒模式
    CAN_InitStructure.CAN_NART = DISABLE;         //不使能非自动重传输模式
    CAN_InitStructure.CAN_RFLM = DISABLE;         //不使能FIFO锁定模式,新的报文直接覆盖旧的报文
    CAN_InitStructure.CAN_TXFP = DISABLE;         //不使能发送FIFO优先级，优先级由报文标识符决定
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; //正常模式
                                                  //设置波特率
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;      //重新同步跳跃宽度 1 个时间单位
    CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq;      //时间段 1 为 5 个时间单位
    CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;      //时间段 2 为 6 个时间单位 250K
    CAN_InitStructure.CAN_Prescaler = 6;          //设定了一个时间单位的长度，它的范围是 1 到 1024。
                                                  // CANbps=APB1总线频率18000000/6/(6+5+1))=250k bps
    CAN_Init(CAN2, &CAN_InitStructure);

#if 1
    CAN_FilterInitStructure.CAN_FilterNumber = 14;                   //指定了待初始化的过滤器，它的范围是 1 到 13
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  //无掩码模式
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // 32 位过滤器

    //对扩展数据帧进行过滤:(只接收扩展数据帧) 扩展ID为29bit
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&CAN_FilterInitStructure);

#endif

    // 使能FIFO0消息挂号中断、FIFO0满中断、FIFO0溢出中断
    CAN_ITConfig(CAN2, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);
    //不使能发送邮箱空中断
    // CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
    //使能错误警告中断、错误被动中断、离线中断、上次错误号中断、错误中断、唤醒中断、睡眠标志位中断
    // CAN_ITConfig(CAN1, CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC
    //                 | CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);    // ERR中断
    // CAN缓存初始化
    //memset(CAN_msg_num,0,MAX_MAIL_NUM);
    _memset(CAN_msg_num, 0, MAX_MAIL_NUM);
    _memset((u8 *)tr9_Set_Can, 0x00, sizeof(Set_Can_t) * 28);
    _memset((u8 *)Can_Id, 0x00, sizeof(CAN_ID_FILTRATION_T) * 14);
}

/********************************************************************************
*函数名称：Can_SetFilterID_Model
*函数功能：可以设置设置CAN的采集通道,数据帧格式,掩码或者列表模式
*函数参数：1,通道，2,过滤器序号,3,模式，4,需要过滤的ID，5掩码
*返 回 值:无
*注意事项：掩码与过滤ID相同，设置列表模式需要设置ID和掩码
*********************************************************************************/
void Can_SetFilterID_Model(u8 CanNumber, u8 MsgId, u8 Mode, u32 FilterID, u32 MaskID) {
    u32 PGN_Check;
    u32 PGN_mask;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    CAN_FilterInitStructure.CAN_FilterNumber = MsgId;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;

    if (Mode == 1) //标准帧
    {
#if 1 //32位掩码模式
        PGN_Check = (u32)(FilterID << 3);
        CAN_FilterInitStructure.CAN_FilterIdHigh = (u16)(PGN_Check >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterIdLow = (u16)(PGN_Check & 0x7ff) | CAN_ID_STD;
        PGN_mask = 0x7ff;
        PGN_mask = (u32)(PGN_mask << 3);
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (u16)(PGN_mask >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = (u16)(PGN_mask & 0x7ff) | CAN_ID_STD;
#endif
    } else if (Mode == 2) //扩展帧
    {
#if 1 //32位掩码模式
        PGN_Check = (u32)(FilterID << 3);
        CAN_FilterInitStructure.CAN_FilterIdHigh = (u16)(PGN_Check >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterIdLow = (u16)(PGN_Check & 0xffff) | CAN_ID_EXT;
        PGN_mask = 0x1fffffff;
        PGN_mask = (u32)(PGN_mask << 3);
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (u16)(PGN_mask >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = (u16)(PGN_mask & 0xffff) | CAN_ID_EXT;
#endif
    } else {
#if 1 //无掩码
        CAN_FilterInitStructure.CAN_FilterIdHigh = 0;
        CAN_FilterInitStructure.CAN_FilterIdLow = 0;
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0;
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
#endif
    }

    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FilterFIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;
    CAN_FilterInit(&CAN_FilterInitStructure);

    /* CAN FIFO0 message pending interrupt enable */

    if (CanNumber == 1)
        CAN_ITConfig(CAN1, CAN_IT_FMP0, ENABLE);
    else
        CAN_ITConfig(CAN2, CAN_IT_FMP0, ENABLE);

    return;
}
#if 0
/********************************************************************************
*函数名称：Can_SetModel
*函数功能：设置单个ID采集和是一路ID采集
*函数参数：
*返 回 值: 无
*备     注：无掩码模式就所有CANId都可以采集,列表模式就只能采集到指定的ID
*********************************************************************************/
void Can_SetModel(u8 Mode)
{
	u8 i;
	u8 CAN_ID_LEN = Can_param_get.ID_Quantity;
	
	if(Mode == 1)							//单个CAN_ID 设置
	{											
		for(i = 0 ; i < CAN_ID_LEN ; i++)		//列表模式		
		{
			if(tr9_Set_Can[i].data_type == 0 )	
			{
				Can_SetFilterID_Model(1,i,1,tr9_Set_Can[i].ID_STD,tr9_Set_Can[i].ID_STD); //标准帧
			}		
			else
			{
				Can_SetFilterID_Model(1,i,2,tr9_Set_Can[i].ID_STD,tr9_Set_Can[i].ID_EXT); //扩展帧
			}
		}
	}
	else										//一路CAN设置
	{
		Can_SetFilterID_Model(1,1,3,0,0);		//无掩码模式
	}

}

#endif

/*
void Can_manage()
{

}
*/

//can_s_cali.open
void test_can_speed(void) {
    CanTxMsg TxMessage;
    static u16 speed = 0;
    uint8_t TransmitMailbox = 0;
    static u8 cnt = 0;

    if (can_s_cali.open) {
        speed = 16000;
        cnt = 10;
    } else {
        speed = 64000;
        if (cnt > 0)
            cnt--;
        if (cnt == 0)
            can_s_cali.swi = false;
    }
    ///扩展帧赋值
    TxMessage.ExtId = extCAN_CONTRAL_SPEED; //TxMessage.StdId
    TxMessage.RTR = CAN_RTR_DATA;
    TxMessage.IDE = CAN_ID_EXT; //CAN_ID_STD;
    TxMessage.DLC = 8;
    TxMessage.Data[0] = 0x03;
    TxMessage.Data[1] = speed;
    TxMessage.Data[2] = speed >> 8;
    TxMessage.Data[3] = 0xff;
    TxMessage.Data[4] = 0xff;
    TxMessage.Data[5] = 0xff;
    TxMessage.Data[6] = 0xff;
    TxMessage.Data[7] = 0xff;

    //TransmitMailbox = CAN_Transmit(CAN1, &TxMessage);

#if 1 //2022-03-17 增加.
    CAN_Transmit(CAN1, &TxMessage);
    CAN_Transmit(CAN2, &TxMessage);
#endif

    if (CAN_NO_MB == TransmitMailbox) {
        return;
    } else {
        CAN_msg_num[TransmitMailbox] = 1;
    }

    //  CAN_ITConfig(CAN1, CAN_IT_TME, ENABLE);
    return;
}

/*****************************************************
* 函数功能：读取车辆速度
* 函数参数：str:CAN读取到的车辆速度
* 创建时间：2021/12/30
* 备		注：无
******************************************************/

void get_wheel_speed_from_can(u8 *str) {
    u16 speed = 0;
    speed = str[2];
    wheel_speed = speed;
    if (wheel_speed > 120)
        wheel_speed = 120;
    if (car_state.bit.acc == false)
        wheel_speed = 0;
}

/***************************************************************************************
* 函数名称：tr9_CAN_Data_Upload
* 函数功能：采集上传CAN数据
* 函数参数：1.功能选择
* 备     注：无
***************************************************************************************/
void tr9_CAN_Data_Upload(u8 coun) {
    //	static enum
    //    {
    //		E_CAN_HEAD,
    //		E_CAN_DATA,
    //    } step = E_CAN_DATA;

    u16 i = 0;
    //	u16 j ;
    //	u8  msms= 0 ;
    static u16 len = 0;

    //static u16 time = 0;
    //	u32 tid = 0;

    //	u8 Can_data[5]={0};
    if (0 == coun) {
    }

    else if (1 == coun) {
        //time = tick ;

        //if(Can_param.CAN1_count != 0)
        if (0 != cansendbuf.cnt) {
            //tr9_show = false;		//关闭tr9数据发送数据显示

            len = 0;

            len += _sw_endian(&CAN_buf[len], (u8 *)&cansendbuf.cnt, 2);

            //len += _memcpy_len(&CAN_buf[len], (u8 *)&cansendbuf.cnt, 2);
            //len += _memcpy_len(&CAN_buf[len], cansendbuf.reception_time, 5);

            Can_id_config.bit.type = 1; //帧类型		0是标准帧
            Can_id_config.bit.mode = 0;

            for (i = 0; i < cansendbuf.cnt; i++) {
                Can_id_config.bit.ID = (cansendbuf.datbuf[i].can_id & 0x1fffffff);

                len += _memcpy_len(&CAN_buf[len], cansendbuf.datbuf[i].time, 5);
                len += _sw_endian(&CAN_buf[len], (u8 *)&Can_id_config.reg, 4);
                len += _memcpy_len(&CAN_buf[len], cansendbuf.datbuf[i].data, 8);
            }

            tr9_frame_pack2rk(tr9_cmd_6055, CAN_buf, len);

            len = 0;
            //tr9_show = true;
            cansendbuf.cnt = 0;
            //Can_param.CAN1_count = 0;
            //_memset(CAN_buf,0x00,sizeof(CAN_buf));

            //step = E_CAN_HEAD;
        }
    }
}

/***************************************************************************************
* 函数名称：Can1_Data_Uploading
* 函数功能：对CAN1数据组包上传
* 函数参数：无
* 时     间:	2022.2.25
* 备     注：无
***************************************************************************************/
void Can1_Data_Uploading(void) {
    u16 len = 0, i;
    if (0 != cansendbuf.cnt) {
        len += _sw_endian(&CAN_buf[len], (u8 *)&cansendbuf.cnt, 2); //数据长度

        Can_id_config.bit.type = 1; //帧类型 	0是标准帧
        Can_id_config.bit.mode = 0; //0是原始数据
        Can_id_config.bit.chnnal = 0;
        for (i = 0; i < cansendbuf.cnt; i++) {
            Can_id_config.bit.ID = (cansendbuf.datbuf[i].can_id & 0x1fffffff);

            len += _memcpy_len(&CAN_buf[len], cansendbuf.datbuf[i].time, 5); //时间
            len += _sw_endian(&CAN_buf[len], (u8 *)&Can_id_config.reg, 4);   //ID
            len += _memcpy_len(&CAN_buf[len], cansendbuf.datbuf[i].data, 8); //数据
        }
        tr9_frame_pack2rk(tr9_cmd_6055, CAN_buf, len);
    }
    cansendbuf.cnt = 0;
}

/***************************************************************************************
* 函数名称：Can2_Data_Uploading
* 函数功能：对CAN2数据组包上传
* 函数参数：无
* 时     间:	2022.2.25
* 备     注：无
***************************************************************************************/
void Can2_Data_Uploading(void) {
    u16 len = 0, i;

    if (0 != cansendbuf2.cnt) {
        len += _sw_endian(&CAN_buf2[len], (u8 *)&cansendbuf2.cnt, 2); //数据长度

        Can_id_config2.bit.type = 1; //帧类型 	0是标准帧
        Can_id_config2.bit.mode = 0; //0是原始数据
        Can_id_config2.bit.chnnal = 1;

        for (i = 0; i < cansendbuf2.cnt; i++) {
            Can_id_config2.bit.ID = (cansendbuf2.datbuf[i].can_id & 0x1fffffff);

            len += _memcpy_len(&CAN_buf2[len], cansendbuf2.datbuf[i].time, 5); //时间
            len += _sw_endian(&CAN_buf2[len], (u8 *)&Can_id_config2.reg, 4);   //ID
            len += _memcpy_len(&CAN_buf2[len], cansendbuf2.datbuf[i].data, 8); //数据
        }

        tr9_frame_pack2rk(tr9_cmd_6055, CAN_buf2, len);
    }

    cansendbuf2.cnt = 0;
}

/***************************************************************************************
* 函数名称：tr9_6037_task_CAN
* 函数功能：CAN的采集/上传时间
* 函数参数：无
* 时     间:	2022.1.8
* 备     注：时间设为零时不采集不上传
***************************************************************************************/
void tr9_6037_task_CAN(void) {
    //	u8 i;
    u32 tid = 0;
    static bool CAN1_flag = false;
    static bool CAN2_flag = false;
    static u32 CAN1_put = 0;
    static u32 CAN2_put = 0;

    if (Can_param.CAN1_data_Flag) //CAN采集到数据	    	/*
    {
        tid = RxMessage.ExtId; //读取接收到扩展帧的ID
        if (tid == 0x18fef100)
            get_wheel_speed_from_can(RxMessage.Data);

        CAN1_flag = true;
        Can_param.CAN1_data_Flag = false;
    }

    if (Can_param.CAN2_data_Flag) {
#if 1                           //2022-03-17 add by hj
        tid = RxMessage2.ExtId; //读取接收到扩展帧的ID
        if (tid == 0x18fef100)
            get_wheel_speed_from_can(RxMessage2.Data);
#endif

        CAN2_flag = true;
        Can_param.CAN2_data_Flag = false;
    }

    if (CAN1_flag == false) //在没有CAN数据之前刷新为实时时间
    {
        CAN1_put = jiffies;
    } else if (0 != Can_param.CAN1_UploadingTime) {
        if (_pastn(CAN1_put) >= Can_param.CAN1_UploadingTime) {
            CAN1_put = jiffies;
            Can_param_get.CAN1_send_flag = true;
            Can1_Data_Uploading(); //CAN1数据打包上传
            Can_param_get.CAN1_send_flag = false;
        }
    }

    if (CAN2_flag == false) {
        CAN2_put = jiffies;
    } else if (0 != Can_param.CAN2_UploadingTime) {
        if (_pastn(CAN2_put) >= Can_param.CAN2_UploadingTime) {
            CAN2_put = jiffies;
            Can_param_get.CAN2_send_flag = true;
            Can2_Data_Uploading(); //CAN2数据打包上传
            Can_param_get.CAN2_send_flag = false;
        }
    }
}

/***************************************************************************************
* 函数名称：Set_Can_ID_Filter
* 函数功能：设置CAN ID 过滤
* 函数参数：无
* 时     间:	2022.2.22
* 备     注：在列表模式下，一共有14个过滤器，一个过滤器可以过滤两个ID，最多可以设置28个ID
*			这是陈工的操作方式
***************************************************************************************/
void Set_Can_ID_Filter(CAN_ID_FILTRATION_T *ID, u8 idx) {
    CAN_FilterInitTypeDef Filter_ID;

    Filter_ID.CAN_FilterNumber = idx;                  //指定了待初始化的过滤器，它的范围是 1 到 13
    Filter_ID.CAN_FilterMode = CAN_FilterMode_IdList;  //标识符列表模式
    Filter_ID.CAN_FilterScale = CAN_FilterScale_32bit; // 32 位过滤器

    //对扩展数据帧进行过滤:(只接收扩展数据帧) 扩展ID为29bit
    Filter_ID.CAN_FilterIdHigh = (((u32)ID->ID1 << 3) & 0xFFFF0000) >> 16;
    Filter_ID.CAN_FilterIdLow = (((u32)ID->ID1 << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;
    Filter_ID.CAN_FilterMaskIdHigh = (((u32)ID->ID2 << 3) & 0xFFFF0000) >> 16;
    Filter_ID.CAN_FilterMaskIdLow = (((u32)ID->ID2 << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;
    Filter_ID.CAN_FilterFIFOAssignment = CAN_FIFO0;
    Filter_ID.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&Filter_ID);
}

/***************************************************************************************
* 函数名称：Set_Can_ID 
* 函数功能：设置好需要过滤的ID，可以使一个过滤器过滤两个ID
* 函数参数：无
* 时     间:	2022.2.22
* 备     注：配个Set_Can_ID_Filter函数使用
***************************************************************************************/

#endif
