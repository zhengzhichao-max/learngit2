#include "include_all.h"

#if (P_CAN == _CAN_BASE)

CanRxMsg RxMessage;  //����CAN1������
CanRxMsg RxMessage2; //����CAN2������

u8 CAN_msg_num[MAX_MAIL_NUM];  // ���������� CAN1
u8 CAN_msg_num2[MAX_MAIL_NUM]; // ���������� CAN2

u8 CAN_buf[CAN_DATA_SIZE] = {0};  //CAN1���ݴ����
u8 CAN_buf2[CAN_DATA_SIZE] = {0}; //CAN2���ݴ����

u16 can_limit_rabio = 0;
//static u8 cun =0;
CAN_param_t Can_param;
CAN_PARAM_GET_T Can_param_get;
CAN_ID_CONFIG_T Can_id_config;  //�����ʹ��
CAN_ID_CONFIG_T Can_id_config2; //�����ʹ��

Set_Can_t tr9_Set_Can[28];       //����CAN������
SOFTEWARE_LOCK_T Wofteware_lock; //�����

static CAN_ID_FILTRATION_T Can_Id[14]; //����������28��CAN_ID

CAN_SEND_DATA cansendbuf;  //����CAN1������
CAN_SEND_DATA cansendbuf2; //����CAN2������

////���Թ������б�ģʽ  ��  ����ģʽ

#if 0
void InitCan(void)
{
	CAN_InitTypeDef			CAN_InitStructure;
	CAN_FilterInitTypeDef	CAN_FilterInitStructure;

	//u32 Id0 = 0x18fef100;    //EEC1��������������
	//u32 Id1 = 0x18fef200;	 //Ѳ�����Ƴ��٣������ź�
	u32 Id0 = 0x18fef100;    //EEC1��������������
	u32 Id1 = 0x18fef200;	 //Ѳ�����Ƴ��٣������ź�


    //������������CAN������
	CAN_DeInit(CAN1);                           //��CAN1���еļĴ�������Ϊȱʡֵ                           
	CAN_StructInit(&CAN_InitStructure);         //��ʼ�� CAN_InitStructure �ṹ��

	CAN_InitStructure.CAN_TTCM=DISABLE;         //��ʹ��ʱ�䴥��ͨѶģʽ
	CAN_InitStructure.CAN_ABOM=ENABLE;          //ʹ���Զ����߹���
	CAN_InitStructure.CAN_AWUM=DISABLE;         //��ʹ���Զ�����ģʽ
	CAN_InitStructure.CAN_NART=DISABLE;         //��ʹ�ܷ��Զ��ش���ģʽ
	CAN_InitStructure.CAN_RFLM=DISABLE;         //��ʹ��FIFO����ģʽ,�µı���ֱ�Ӹ��Ǿɵı���
	CAN_InitStructure.CAN_TXFP=DISABLE;         //��ʹ�ܷ���FIFO���ȼ������ȼ��ɱ��ı�ʶ������ 
	CAN_InitStructure.CAN_Mode=CAN_Mode_Normal; //����ģʽ 
    //���ò�����
	CAN_InitStructure.CAN_SJW=CAN_SJW_1tq;      //����ͬ����Ծ��� 1 ��ʱ�䵥λ
	CAN_InitStructure.CAN_BS1=CAN_BS1_5tq;	    //ʱ��� 1 Ϊ 5 ��ʱ�䵥λ
	CAN_InitStructure.CAN_BS2=CAN_BS2_6tq;	    //ʱ��� 2 Ϊ 6 ��ʱ�䵥λ 250K
	CAN_InitStructure.CAN_Prescaler=6; 	        //�趨��һ��ʱ�䵥λ�ĳ��ȣ����ķ�Χ�� 1 �� 1024��	
	                       						// CANbps=APB1����Ƶ��18000000/6/(6+5+1))=250k bps  
	CAN_Init(CAN1,&CAN_InitStructure);

#if 1
	CAN_FilterInitStructure.CAN_FilterNumber = 0;                       //ָ���˴���ʼ���Ĺ����������ķ�Χ�� 1 �� 13  
	CAN_FilterInitStructure.CAN_FilterMode   = CAN_FilterMode_IdList;   //��ʶ���б�ģʽ
	CAN_FilterInitStructure.CAN_FilterScale  = CAN_FilterScale_32bit;   // 32 λ������

	//����չ����֡���й���:(ֻ������չ����֡) ��չIDΪ29bit
	CAN_FilterInitStructure.CAN_FilterIdHigh = 		(((u32)Id0<<3)&0xFFFF0000)>>16;
	CAN_FilterInitStructure.CAN_FilterIdLow  = 		(((u32)Id0<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 	(((u32)Id1<<3)&0xFFFF0000)>>16;
	CAN_FilterInitStructure.CAN_FilterMaskIdLow  = 	(((u32)Id1<<3)|CAN_ID_EXT|CAN_RTR_DATA)&0xFFFF;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
	CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

	CAN_FilterInit(&CAN_FilterInitStructure);

#endif
	
	// ʹ��FIFO0��Ϣ�Һ��жϡ�FIFO0���жϡ�FIFO0����ж�
	CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);  
	//��ʹ�ܷ���������ж�
   // CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);       
    //ʹ�ܴ��󾯸��жϡ����󱻶��жϡ������жϡ��ϴδ�����жϡ������жϡ������жϡ�˯�߱�־λ�ж�
   // CAN_ITConfig(CAN1, CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC 
    //                 | CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);    // ERR�ж�
    // CAN�����ʼ��
    //memset(CAN_msg_num,0,MAX_MAIL_NUM);
    _memset(CAN_msg_num,0,MAX_MAIL_NUM);
	_memset(tr9_Set_Can, 0x00, sizeof(Set_Can_t)*28);
	_memset(CAN_ID_FILTRATION_T, 0x00, sizeof(CAN_ID_FILTRATION_T)*28);
}

#endif
const u32 extCAN_CONTRAL_SPEED = {0x0C000003};

/***************************************************************************************
* �������ƣ�InitCan
* �������ܣ���ʼ��CAN1
* ������������1
* ʱ     ��:	2022.2.25
* ��     ע��CAN_FilterMode_IdMask ���������ģʽ				//�����ģʽ��ȫ��ID�����ݶ��ܽ���
*			CAN_FilterMode_IdList ������б�ģʽ			//�����ģʽ��ֻ���յ��б��е�ID
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

    //u32 Id0 = 0x18fef100;    //EEC1��������������
    //u32 Id1 = 0x18fef200;	 //Ѳ�����Ƴ��٣������ź�
    //u32 Id0 = 0x18fef100;    //EEC1��������������
    //u32 Id1 = 0x18fef200;	 //Ѳ�����Ƴ��٣������ź�

    //������������CAN������
    CAN_DeInit(CAN1);                   //��CAN1���еļĴ�������Ϊȱʡֵ
    CAN_StructInit(&CAN_InitStructure); //��ʼ�� CAN_InitStructure �ṹ��

    CAN_InitStructure.CAN_TTCM = DISABLE;         //��ʹ��ʱ�䴥��ͨѶģʽ
    CAN_InitStructure.CAN_ABOM = ENABLE;          //ʹ���Զ����߹���
    CAN_InitStructure.CAN_AWUM = DISABLE;         //��ʹ���Զ�����ģʽ
    CAN_InitStructure.CAN_NART = DISABLE;         //��ʹ�ܷ��Զ��ش���ģʽ
    CAN_InitStructure.CAN_RFLM = DISABLE;         //��ʹ��FIFO����ģʽ,�µı���ֱ�Ӹ��Ǿɵı���
    CAN_InitStructure.CAN_TXFP = DISABLE;         //��ʹ�ܷ���FIFO���ȼ������ȼ��ɱ��ı�ʶ������
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; //����ģʽ
        //���ò�����
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq; //����ͬ����Ծ��� 1 ��ʱ�䵥λ
    CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq; //ʱ��� 1 Ϊ 5 ��ʱ�䵥λ
    CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq; //ʱ��� 2 Ϊ 6 ��ʱ�䵥λ 250K
    CAN_InitStructure.CAN_Prescaler = 6;     //�趨��һ��ʱ�䵥λ�ĳ��ȣ����ķ�Χ�� 1 �� 1024��
                                             // CANbps=APB1����Ƶ��18000000/6/(6+5+1))=250k bps
    CAN_Init(CAN1, &CAN_InitStructure);

#if 1
    CAN_FilterInitStructure.CAN_FilterNumber = 1;                    //ָ���˴���ʼ���Ĺ����������ķ�Χ�� 0 �� 13
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  //������ģʽ
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // 32 λ������

    //����չ����֡���й���:(ֻ������չ����֡) ��չIDΪ29bit
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&CAN_FilterInitStructure);

#endif

    // ʹ��FIFO0��Ϣ�Һ��жϡ�FIFO0���жϡ�FIFO0����ж�
    CAN_ITConfig(CAN1, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);
    //��ʹ�ܷ���������ж�
    // CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
    //ʹ�ܴ��󾯸��жϡ����󱻶��жϡ������жϡ��ϴδ�����жϡ������жϡ������жϡ�˯�߱�־λ�ж�
    // CAN_ITConfig(CAN1, CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC
    //                 | CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);    // ERR�ж�
    // CAN�����ʼ��
    //memset(CAN_msg_num,0,MAX_MAIL_NUM);
    _memset(CAN_msg_num, 0, MAX_MAIL_NUM);
    _memset((u8 *)tr9_Set_Can, 0x00, sizeof(Set_Can_t) * 28);
    _memset((u8 *)Can_Id, 0x00, sizeof(CAN_ID_FILTRATION_T) * 14);
}

void InitCan2(void) {
    CAN_InitTypeDef CAN_InitStructure;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    //u32 Id0 = 0x18fef100;    //EEC1��������������
    //u32 Id1 = 0x18fef200;	 //Ѳ�����Ƴ��٣������ź�
    //u32 Id0 = 0x18fef100;    //EEC1��������������
    //u32 Id1 = 0x18fef200;	 //Ѳ�����Ƴ��٣������ź�

    //������������CAN������
    CAN_DeInit(CAN2);                   //��CAN1���еļĴ�������Ϊȱʡֵ
    CAN_StructInit(&CAN_InitStructure); //��ʼ�� CAN_InitStructure �ṹ��

    CAN_InitStructure.CAN_TTCM = DISABLE;         //��ʹ��ʱ�䴥��ͨѶģʽ
    CAN_InitStructure.CAN_ABOM = ENABLE;          //ʹ���Զ����߹���
    CAN_InitStructure.CAN_AWUM = DISABLE;         //��ʹ���Զ�����ģʽ
    CAN_InitStructure.CAN_NART = DISABLE;         //��ʹ�ܷ��Զ��ش���ģʽ
    CAN_InitStructure.CAN_RFLM = DISABLE;         //��ʹ��FIFO����ģʽ,�µı���ֱ�Ӹ��Ǿɵı���
    CAN_InitStructure.CAN_TXFP = DISABLE;         //��ʹ�ܷ���FIFO���ȼ������ȼ��ɱ��ı�ʶ������
    CAN_InitStructure.CAN_Mode = CAN_Mode_Normal; //����ģʽ
                                                  //���ò�����
    CAN_InitStructure.CAN_SJW = CAN_SJW_1tq;      //����ͬ����Ծ��� 1 ��ʱ�䵥λ
    CAN_InitStructure.CAN_BS1 = CAN_BS1_5tq;      //ʱ��� 1 Ϊ 5 ��ʱ�䵥λ
    CAN_InitStructure.CAN_BS2 = CAN_BS2_6tq;      //ʱ��� 2 Ϊ 6 ��ʱ�䵥λ 250K
    CAN_InitStructure.CAN_Prescaler = 6;          //�趨��һ��ʱ�䵥λ�ĳ��ȣ����ķ�Χ�� 1 �� 1024��
                                                  // CANbps=APB1����Ƶ��18000000/6/(6+5+1))=250k bps
    CAN_Init(CAN2, &CAN_InitStructure);

#if 1
    CAN_FilterInitStructure.CAN_FilterNumber = 14;                   //ָ���˴���ʼ���Ĺ����������ķ�Χ�� 1 �� 13
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;  //������ģʽ
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit; // 32 λ������

    //����չ����֡���й���:(ֻ������չ����֡) ��չIDΪ29bit
    CAN_FilterInitStructure.CAN_FilterIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdHigh = 0;
    CAN_FilterInitStructure.CAN_FilterMaskIdLow = 0;
    CAN_FilterInitStructure.CAN_FilterFIFOAssignment = CAN_FIFO0;
    CAN_FilterInitStructure.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&CAN_FilterInitStructure);

#endif

    // ʹ��FIFO0��Ϣ�Һ��жϡ�FIFO0���жϡ�FIFO0����ж�
    CAN_ITConfig(CAN2, CAN_IT_FMP0 | CAN_IT_FF0 | CAN_IT_FOV0, ENABLE);
    //��ʹ�ܷ���������ж�
    // CAN_ITConfig(CAN1, CAN_IT_TME, DISABLE);
    //ʹ�ܴ��󾯸��жϡ����󱻶��жϡ������жϡ��ϴδ�����жϡ������жϡ������жϡ�˯�߱�־λ�ж�
    // CAN_ITConfig(CAN1, CAN_IT_EWG | CAN_IT_EPV | CAN_IT_BOF | CAN_IT_LEC
    //                 | CAN_IT_ERR | CAN_IT_WKU | CAN_IT_SLK, ENABLE);    // ERR�ж�
    // CAN�����ʼ��
    //memset(CAN_msg_num,0,MAX_MAIL_NUM);
    _memset(CAN_msg_num, 0, MAX_MAIL_NUM);
    _memset((u8 *)tr9_Set_Can, 0x00, sizeof(Set_Can_t) * 28);
    _memset((u8 *)Can_Id, 0x00, sizeof(CAN_ID_FILTRATION_T) * 14);
}

/********************************************************************************
*�������ƣ�Can_SetFilterID_Model
*�������ܣ�������������CAN�Ĳɼ�ͨ��,����֡��ʽ,��������б�ģʽ
*����������1,ͨ����2,���������,3,ģʽ��4,��Ҫ���˵�ID��5����
*�� �� ֵ:��
*ע��������������ID��ͬ�������б�ģʽ��Ҫ����ID������
*********************************************************************************/
void Can_SetFilterID_Model(u8 CanNumber, u8 MsgId, u8 Mode, u32 FilterID, u32 MaskID) {
    u32 PGN_Check;
    u32 PGN_mask;
    CAN_FilterInitTypeDef CAN_FilterInitStructure;

    CAN_FilterInitStructure.CAN_FilterNumber = MsgId;
    CAN_FilterInitStructure.CAN_FilterMode = CAN_FilterMode_IdMask;
    CAN_FilterInitStructure.CAN_FilterScale = CAN_FilterScale_32bit;

    if (Mode == 1) //��׼֡
    {
#if 1 //32λ����ģʽ
        PGN_Check = (u32)(FilterID << 3);
        CAN_FilterInitStructure.CAN_FilterIdHigh = (u16)(PGN_Check >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterIdLow = (u16)(PGN_Check & 0x7ff) | CAN_ID_STD;
        PGN_mask = 0x7ff;
        PGN_mask = (u32)(PGN_mask << 3);
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (u16)(PGN_mask >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = (u16)(PGN_mask & 0x7ff) | CAN_ID_STD;
#endif
    } else if (Mode == 2) //��չ֡
    {
#if 1 //32λ����ģʽ
        PGN_Check = (u32)(FilterID << 3);
        CAN_FilterInitStructure.CAN_FilterIdHigh = (u16)(PGN_Check >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterIdLow = (u16)(PGN_Check & 0xffff) | CAN_ID_EXT;
        PGN_mask = 0x1fffffff;
        PGN_mask = (u32)(PGN_mask << 3);
        CAN_FilterInitStructure.CAN_FilterMaskIdHigh = (u16)(PGN_mask >> 16) & 0xffff;
        CAN_FilterInitStructure.CAN_FilterMaskIdLow = (u16)(PGN_mask & 0xffff) | CAN_ID_EXT;
#endif
    } else {
#if 1 //������
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
*�������ƣ�Can_SetModel
*�������ܣ����õ���ID�ɼ�����һ·ID�ɼ�
*����������
*�� �� ֵ: ��
*��     ע��������ģʽ������CANId�����Բɼ�,�б�ģʽ��ֻ�ܲɼ���ָ����ID
*********************************************************************************/
void Can_SetModel(u8 Mode)
{
	u8 i;
	u8 CAN_ID_LEN = Can_param_get.ID_Quantity;
	
	if(Mode == 1)							//����CAN_ID ����
	{											
		for(i = 0 ; i < CAN_ID_LEN ; i++)		//�б�ģʽ		
		{
			if(tr9_Set_Can[i].data_type == 0 )	
			{
				Can_SetFilterID_Model(1,i,1,tr9_Set_Can[i].ID_STD,tr9_Set_Can[i].ID_STD); //��׼֡
			}		
			else
			{
				Can_SetFilterID_Model(1,i,2,tr9_Set_Can[i].ID_STD,tr9_Set_Can[i].ID_EXT); //��չ֡
			}
		}
	}
	else										//һ·CAN����
	{
		Can_SetFilterID_Model(1,1,3,0,0);		//������ģʽ
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
    ///��չ֡��ֵ
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

#if 1 //2022-03-17 ����.
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
* �������ܣ���ȡ�����ٶ�
* ����������str:CAN��ȡ���ĳ����ٶ�
* ����ʱ�䣺2021/12/30
* ��		ע����
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
* �������ƣ�tr9_CAN_Data_Upload
* �������ܣ��ɼ��ϴ�CAN����
* ����������1.����ѡ��
* ��     ע����
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
            //tr9_show = false;		//�ر�tr9���ݷ���������ʾ

            len = 0;

            len += _sw_endian(&CAN_buf[len], (u8 *)&cansendbuf.cnt, 2);

            //len += _memcpy_len(&CAN_buf[len], (u8 *)&cansendbuf.cnt, 2);
            //len += _memcpy_len(&CAN_buf[len], cansendbuf.reception_time, 5);

            Can_id_config.bit.type = 1; //֡����		0�Ǳ�׼֡
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
* �������ƣ�Can1_Data_Uploading
* �������ܣ���CAN1��������ϴ�
* ������������
* ʱ     ��:	2022.2.25
* ��     ע����
***************************************************************************************/
void Can1_Data_Uploading(void) {
    u16 len = 0, i;
    if (0 != cansendbuf.cnt) {
        len += _sw_endian(&CAN_buf[len], (u8 *)&cansendbuf.cnt, 2); //���ݳ���

        Can_id_config.bit.type = 1; //֡���� 	0�Ǳ�׼֡
        Can_id_config.bit.mode = 0; //0��ԭʼ����
        Can_id_config.bit.chnnal = 0;
        for (i = 0; i < cansendbuf.cnt; i++) {
            Can_id_config.bit.ID = (cansendbuf.datbuf[i].can_id & 0x1fffffff);

            len += _memcpy_len(&CAN_buf[len], cansendbuf.datbuf[i].time, 5); //ʱ��
            len += _sw_endian(&CAN_buf[len], (u8 *)&Can_id_config.reg, 4);   //ID
            len += _memcpy_len(&CAN_buf[len], cansendbuf.datbuf[i].data, 8); //����
        }
        tr9_frame_pack2rk(tr9_cmd_6055, CAN_buf, len);
    }
    cansendbuf.cnt = 0;
}

/***************************************************************************************
* �������ƣ�Can2_Data_Uploading
* �������ܣ���CAN2��������ϴ�
* ������������
* ʱ     ��:	2022.2.25
* ��     ע����
***************************************************************************************/
void Can2_Data_Uploading(void) {
    u16 len = 0, i;

    if (0 != cansendbuf2.cnt) {
        len += _sw_endian(&CAN_buf2[len], (u8 *)&cansendbuf2.cnt, 2); //���ݳ���

        Can_id_config2.bit.type = 1; //֡���� 	0�Ǳ�׼֡
        Can_id_config2.bit.mode = 0; //0��ԭʼ����
        Can_id_config2.bit.chnnal = 1;

        for (i = 0; i < cansendbuf2.cnt; i++) {
            Can_id_config2.bit.ID = (cansendbuf2.datbuf[i].can_id & 0x1fffffff);

            len += _memcpy_len(&CAN_buf2[len], cansendbuf2.datbuf[i].time, 5); //ʱ��
            len += _sw_endian(&CAN_buf2[len], (u8 *)&Can_id_config2.reg, 4);   //ID
            len += _memcpy_len(&CAN_buf2[len], cansendbuf2.datbuf[i].data, 8); //����
        }

        tr9_frame_pack2rk(tr9_cmd_6055, CAN_buf2, len);
    }

    cansendbuf2.cnt = 0;
}

/***************************************************************************************
* �������ƣ�tr9_6037_task_CAN
* �������ܣ�CAN�Ĳɼ�/�ϴ�ʱ��
* ������������
* ʱ     ��:	2022.1.8
* ��     ע��ʱ����Ϊ��ʱ���ɼ����ϴ�
***************************************************************************************/
void tr9_6037_task_CAN(void) {
    //	u8 i;
    u32 tid = 0;
    static bool CAN1_flag = false;
    static bool CAN2_flag = false;
    static u32 CAN1_put = 0;
    static u32 CAN2_put = 0;

    if (Can_param.CAN1_data_Flag) //CAN�ɼ�������	    	/*
    {
        tid = RxMessage.ExtId; //��ȡ���յ���չ֡��ID
        if (tid == 0x18fef100)
            get_wheel_speed_from_can(RxMessage.Data);

        CAN1_flag = true;
        Can_param.CAN1_data_Flag = false;
    }

    if (Can_param.CAN2_data_Flag) {
#if 1                           //2022-03-17 add by hj
        tid = RxMessage2.ExtId; //��ȡ���յ���չ֡��ID
        if (tid == 0x18fef100)
            get_wheel_speed_from_can(RxMessage2.Data);
#endif

        CAN2_flag = true;
        Can_param.CAN2_data_Flag = false;
    }

    if (CAN1_flag == false) //��û��CAN����֮ǰˢ��Ϊʵʱʱ��
    {
        CAN1_put = jiffies;
    } else if (0 != Can_param.CAN1_UploadingTime) {
        if (_pastn(CAN1_put) >= Can_param.CAN1_UploadingTime) {
            CAN1_put = jiffies;
            Can_param_get.CAN1_send_flag = true;
            Can1_Data_Uploading(); //CAN1���ݴ���ϴ�
            Can_param_get.CAN1_send_flag = false;
        }
    }

    if (CAN2_flag == false) {
        CAN2_put = jiffies;
    } else if (0 != Can_param.CAN2_UploadingTime) {
        if (_pastn(CAN2_put) >= Can_param.CAN2_UploadingTime) {
            CAN2_put = jiffies;
            Can_param_get.CAN2_send_flag = true;
            Can2_Data_Uploading(); //CAN2���ݴ���ϴ�
            Can_param_get.CAN2_send_flag = false;
        }
    }
}

/***************************************************************************************
* �������ƣ�Set_Can_ID_Filter
* �������ܣ�����CAN ID ����
* ������������
* ʱ     ��:	2022.2.22
* ��     ע�����б�ģʽ�£�һ����14����������һ�����������Թ�������ID������������28��ID
*			���ǳ¹��Ĳ�����ʽ
***************************************************************************************/
void Set_Can_ID_Filter(CAN_ID_FILTRATION_T *ID, u8 idx) {
    CAN_FilterInitTypeDef Filter_ID;

    Filter_ID.CAN_FilterNumber = idx;                  //ָ���˴���ʼ���Ĺ����������ķ�Χ�� 1 �� 13
    Filter_ID.CAN_FilterMode = CAN_FilterMode_IdList;  //��ʶ���б�ģʽ
    Filter_ID.CAN_FilterScale = CAN_FilterScale_32bit; // 32 λ������

    //����չ����֡���й���:(ֻ������չ����֡) ��չIDΪ29bit
    Filter_ID.CAN_FilterIdHigh = (((u32)ID->ID1 << 3) & 0xFFFF0000) >> 16;
    Filter_ID.CAN_FilterIdLow = (((u32)ID->ID1 << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;
    Filter_ID.CAN_FilterMaskIdHigh = (((u32)ID->ID2 << 3) & 0xFFFF0000) >> 16;
    Filter_ID.CAN_FilterMaskIdLow = (((u32)ID->ID2 << 3) | CAN_ID_EXT | CAN_RTR_DATA) & 0xFFFF;
    Filter_ID.CAN_FilterFIFOAssignment = CAN_FIFO0;
    Filter_ID.CAN_FilterActivation = ENABLE;

    CAN_FilterInit(&Filter_ID);
}

/***************************************************************************************
* �������ƣ�Set_Can_ID 
* �������ܣ����ú���Ҫ���˵�ID������ʹһ����������������ID
* ������������
* ʱ     ��:	2022.2.22
* ��     ע�����Set_Can_ID_Filter����ʹ��
***************************************************************************************/

#endif
