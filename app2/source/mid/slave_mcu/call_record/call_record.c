#include "include_all.h"



//********************************************************************
//功能:存储通话记录对应项里面的号码总数，姓名，号码
//addr:通话记录里各对应项数据存放地址
//phbk_data:电话本结构体指针
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
//功能:从对应的通话记录选项地址中取出存放的电话类型，姓名，号码到结构体中
//call_record_type:通话记录中对应的选项
//call_record:定义的结构体
//*********************************************************************

u8 call_record_load_index(call_record_type_enum  call_record_type,call_record_struct call_record)
{
	u32  addr;


	switch (call_record_type)
	{
	case call_out_type:     //已拨电话
		addr = flash_call_out_record_addr;
		break;

	case received_call_type:   //已接电话
		addr = flash_received_call_record_addr;
		break;

	case missed_call_type:  //未接电话
		addr = flash_missed_call_record_addr;
		break;

	default:
		return	0x00;  //异常数据，直接跳出函数
	}

	spi_flash_read(((u8*)(&call_record)), addr, (sizeof(call_record_struct)));
	
	if ((call_record.call_record_cnt)>call_record_num)
		(call_record.call_record_cnt) = call_record_num;

	return	call_record.call_record_cnt;
}
#if 0

	//生成通话记录信息菜单函数
	//参数ptr表格指针
	//参数d_buff生成菜单缓冲区
	//参数lgth有效信息数
	//参数posi为加载数据的位置
	//参数cnt为加载数据条数计数
	//参数函数返回值	未读信息数
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
名	  称：data_mask_ascii
功	  能：将一个16进制数据转换成一串ASCII数据
参	  数：
输	  出：
编写日期：2012-08-16
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




