#include "include_all.h"



//********************************************************************
//����:�洢ͨ����¼��Ӧ������ĺ�������������������
//addr:ͨ����¼�����Ӧ�����ݴ�ŵ�ַ
//phbk_data:�绰���ṹ��ָ��
//*********************************************************************
void write_data_to_call_record_flash(u32 addr,phbk_data_struct  phbk_data)
{
	_memset((void *)&call_record,0x00,sizeof(call_record));
	spi_flash_read((u8 *)&call_record, addr, sizeof(call_record_struct));
	
	call_record.call_record_cnt=call_record.call_record_cnt%call_record_num;
	call_record.call_record_cnt++;
	call_record.call_record_buff[call_record_num-call_record.call_record_cnt].type=phbk_data.type;
	call_record.call_record_buff[call_record_num-call_record.call_record_cnt].name.lgth=phbk_data.name.lgth;
	_memcpy(call_record.call_record_buff[call_record_num-call_record.call_record_cnt].name.buff,phbk_data.name.buff,phbk_data.name.lgth);
	call_record.call_record_buff[call_record_num-call_record.call_record_cnt].num.lgth=phbk_data.num.lgth;
	_memcpy(call_record.call_record_buff[call_record_num-call_record.call_record_cnt].num.buff,phbk_data.num.buff,phbk_data.num.lgth);

	//spi_flash_write((u8 *)(&call_record.call_record_buff[call_record_num-call_record.call_record_cnt]),addr+1+(call_record_num-call_record.call_record_cnt)*sizeof(call_record_sub_struct),sizeof(call_record_sub_struct));
	//spi_flash_write(&call_record.call_record_cnt,addr,sizeof(call_record.call_record_cnt));
	flash25_program_auto_save(addr,((u8 *)(&call_record)),sizeof(call_record_struct));
}

//***********************************************************************
//����:�Ӷ�Ӧ��ͨ����¼ѡ���ַ��ȡ����ŵĵ绰���ͣ����������뵽�ṹ����
//call_record_type:ͨ����¼�ж�Ӧ��ѡ��
//call_record:����Ľṹ��
//*********************************************************************

u8 call_record_load_index(call_record_type_enum  call_record_type,call_record_struct call_record)
{
	u32  addr;


	switch (call_record_type)
	{
	case call_out_type:     //�Ѳ��绰
		addr = flash_call_out_record_addr;
		break;

	case received_call_type:   //�ѽӵ绰
		addr = flash_received_call_record_addr;
		break;

	case missed_call_type:  //δ�ӵ绰
		addr = flash_missed_call_record_addr;
		break;

	default:
		return	0x00;  //�쳣���ݣ�ֱ����������
	}

	spi_flash_read(((u8*)(&call_record)), addr, (sizeof(call_record_struct)));
	
	if ((call_record.call_record_cnt)>call_record_num)
		(call_record.call_record_cnt) = call_record_num;

	return	call_record.call_record_cnt;
}
#if 0

	//����ͨ����¼��Ϣ�˵�����
	//����ptr���ָ��
	//����d_buff���ɲ˵�������
	//����lgth��Ч��Ϣ��
	//����posiΪ�������ݵ�λ��
	//����cntΪ����������������
	//������������ֵ	δ����Ϣ��
	//
void  call_record_dis_build(u8 d_buff,u8 *s_buff[] ,u8 lgth)
	{
		u8 i,j;


	 if(lgth<3)
	 	{j=lgth;}
	 else
	 	{j=3;}
		for(i=0x00; i<j; i++)
		{

				*(*(s_buff+i)) =((d_buff+i)/10+'0');
				*(*(s_buff+i )+1)=((d_buff+i)%10+'0');
				*(*(s_buff+i)+2 )= '.';
		}
	}



	void  call_record_dis_build(u8 d_buff,u8 *s_buff[] ,u8 lgth)
		{
		if(lgth==1)
			{
			   s_buff[0][0]=((d_buff+1)/10+'0');
			   s_buff[0][1]=((d_buff+1)%10+'0');
     		   s_buff[0][2]='.';
			}
		else if(lgth==2)
			{
			   s_buff[0][0]=((d_buff+1)/10+'0');
			   s_buff[0][1]=((d_buff+1)%10+'0');
     		   s_buff[0][2]='.';
			   s_buff[1][0]=((d_buff+2)/10+'0');
			   s_buff[1][1]=((d_buff+2)%10+'0');
     		   s_buff[1][2]='.';
			}
		else
			{
			   s_buff[0][0]=((d_buff+1)/10+'0');
			   s_buff[0][1]=((d_buff+1)%10+'0');
     		   s_buff[0][2]='.';
			   s_buff[1][0]=((d_buff+2)/10+'0');
			   s_buff[1][1]=((d_buff+2)%10+'0');
     		   s_buff[1][2]='.';
			   s_buff[2][0]=((d_buff+3)/10+'0');
			   s_buff[2][1]=((d_buff+3)%10+'0');
     		   s_buff[2][2]='.';
			}







		}
#endif


/*********************************************************
��	  �ƣ�data_mask_ascii
��	  �ܣ���һ��16��������ת����һ��ASCII����
��	  ����
��	  ����
��д���ڣ�2012-08-16
*********************************************************/
void  call_record_dis_build(u8* dest, u32 s_data, u8 len)
{
	while (len--)
	{
		dest[len] = ((s_data%10) + '0');
		s_data /= 10;
	}
	*(dest+2)='.';
}




