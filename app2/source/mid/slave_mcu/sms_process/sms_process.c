#include "include_all.h"






const u8 *sms_lib_dis[]=
{
	"�Ѷ�",
	"δ��",
};







//��Ϣ������ʼ��
//����ptrΪ��Ϣ����
//����base_addrΪ��ǰ��������Ϣ�洢����ַ
//����single_lgthΪ��ǰ��������Ϣ������Ϣ����
//
static void sms_index_init(sms_index_struct* ptr, u32 base_addr, u16 single_lgth)
{
	u8  i;
	u32  temp;

	ptr->total = 0x00;  //��������
	temp = base_addr+sms_index_content_lgth;

	for (i=0x00; i<sms_total_max; i++)
	{
		((ptr->use_infor)+i)->addr = temp;
		temp += single_lgth;
	}
}




//��Ϣ���FLASH���ݳ�ʼ��
//
void sms_data_flash_init(void)
{
	sms_index_struct  sms_index;

	memset(((uchar*)(&sms_index)), '\x0', (sizeof(sms_index)));

	feed_wdt;
	sms_index_init((&sms_index), flash_sms_center_addr, (sizeof(sms_center_content_struct)));
	flash25_program_auto_save(flash_sms_center_addr, ((uchar*)(&sms_index)), (sizeof(sms_index)));

	sms_index_init((&sms_index), flash_sms_phone_addr, (sizeof(sms_serve_content_struct)));
	flash25_program_auto_save(flash_sms_phone_addr, ((uchar*)(&sms_index)), (sizeof(sms_index)));

	feed_wdt;
	sms_index_init((&sms_index), flash_sms_serve_addr, (sizeof(sms_fix_content_struct)));
	flash25_program_auto_save(flash_sms_serve_addr, ((uchar*)(&sms_index)), (sizeof(sms_index)));

	sms_index_init((&sms_index), flash_sms_fix_addr, (sizeof(sms_phone_content_struct)));
	flash25_program_auto_save(flash_sms_fix_addr, ((uchar*)(&sms_index)), (sizeof(sms_index)));

	feed_wdt;
	sms_index_init((&sms_index), flash_sms_vod_addr, (sizeof(sms_vod_content_struct)));
	flash25_program_auto_save(flash_sms_vod_addr, ((uchar*)(&sms_index)), (sizeof(sms_index)));

	sms_index_init((&sms_index), flash_sms_ask_addr, (sizeof(sms_ask_content_struct)));
	flash25_program_auto_save(flash_sms_ask_addr, ((uchar*)(&sms_index)), (sizeof(sms_index)));
}




//������Ϣ�ڱ���е�λ��
//����ptrΪ���ָ��
//����typeΪ���ԡ�ID�š����͵�����ֵ
//����cntΪ��ǰ���д��ڵ���Ч��ַ������
//��������ֵ    ������Ҫ���ҵ�����ֵ�ڱ���е�λ�ã���Χ0xff��ʾδ�ҵ�
//
static u8 sms_search(use_infor_struct* ptr, u16 type, u8 cnt)
{
	u8  i;

	if (cnt > sms_total_max) //������Χ�����账��
		return  0xff;

	if (cnt == 0x00)         //�ռ�գ����账��
		return  0xff;

	for (i=0x00; i<cnt; i++)
	{
		if (((ptr+i)->type_id_serial_num) == type) //�ҵ�����ֵ
		{
			return  i;
		}
	}

	return  0xff;
}

//���һ����Ϣ
//����ptrΪ���ָ��
//����typeΪ���ԡ�ID�š����͵�����ֵ
//����dat_lgthΪ���������Ϣ����
//����dat_buffΪ���������Ϣ
//
static void sms_add(use_infor_struct* ptr, u16 type, u16 dat_lgth, u8* dat_buff)
{
	u32  temp;  //��ʱ��ַ����

	temp = (ptr+(sms_total_max-1))->addr;  //ȡ����������һ��Ԫ���еĵ�ַ
	array_move(ptr, (sms_total_max-1), true, array_sms_use_move_right);  //����

	(ptr+0)->read_flag = true;  //�´����ֵʼ����δ��״̬
	(ptr+0)->type_id_serial_num = type;
	(ptr+0)->addr = temp;

	flash25_program_auto_save(temp, dat_buff, dat_lgth);  //������Ϣ����
}

//ɾ��һ����Ϣ
//����ptrΪ���ָ��
//����typeΪ���ԡ�ID�š����͵�����ֵ
//����cntΪ��ǰ���д��ڵ���Ч��ַ������
//��������ֵ    ɾ�����    TRUE��ʾɾ���ɹ�    FALSE��ʾɾ��ʧ��
//
static bool sms_del(use_infor_struct* ptr, u16 type, u8 cnt)
{
	u8  i;
	u32  temp;  //��ʱ��ַ����

	i = sms_search(ptr, type, cnt);
	if (i == 0xff) //����������ֵ
	{
		return false;
	}

	temp = (ptr+i)->addr;    //��ȡ��ɾ��������
	array_move((ptr+i+1), (cnt-1-i), false, array_sms_use_move_left);  //����
	(ptr+(cnt-1))->addr = temp;  //ɾ�����ĵ�ַ���������Ŷ�

	return true;
}

//�޸�ԭ�е���Ϣ
//����ptrΪ���ָ��
//����typeΪ���ԡ�ID�š����͵�����ֵ
//����cntΪ��ǰ���д��ڵ���Ч��ַ������
//����dat_lgthΪ���������Ϣ����
//����dat_buffΪ���������Ϣ
//��������ֵ    ɾ�����    TRUE��ʾɾ���ɹ�    FALSE��ʾɾ��ʧ��
//
static bool sms_change(use_infor_struct* ptr, u16 type, u8 cnt, u16 dat_lgth, u8* dat_buff)
{
	u8  i;

	i = sms_search(ptr, type, cnt);
	if (i == 0xff) //����������ֵ
	{
		return false;
	}

	((ptr+i)->read_flag) = true;  //���޸ĵ���Ϣ����Ϊδ��
	flash25_program_auto_save(((ptr+i)->addr), dat_buff, dat_lgth);  //������Ϣ����

	return true;
}




//��Ϣ����������  �������ӡ��޸ġ�ɾ��������Ϣ�Ĳ���
//����ptrΪ��Ϣ����
//����opΪ��ǰ�Ĳ������ͣ�������
//����typeΪ���ԡ�ID�š����͵���Ϣ
//����ct_lgthΪ��Ϣ�����ݵĳ��ȣ���λ�ֽ�
//����ct_ptrΪ��Ϣ������ת��Ϊ�ַ���֮���ָ��
//��������ֵ    �������    TRUE��ʾ�����ɹ�    FALSE��ʾ����ʧ��
//
bool sms_operate_hdl(sms_index_struct* ptr, sms_op_enum op, u16 type, u16 ct_lgth, u8* ct_ptr)
{
	switch (op)
	{
	case sms_op_add:  //����
		sms_add((ptr->use_infor), type, ct_lgth, ct_ptr);
		(ptr->total) += 1;
		break;

	case sms_op_del:  //ɾ��
		if (sms_del((ptr->use_infor), type, (ptr->total)) == false)
		{
			return  false;
		}

		(ptr->total) -= 1;
		break;

	case sms_op_change:  //�޸�
		if (sms_change((ptr->use_infor), type, (ptr->total), ct_lgth, ct_ptr) == false)
		{
			return  false;
		}
		break;
	}

	if ((ptr->total)>sms_total_max)
		(ptr->total) = sms_total_max;

	return  true;
}








