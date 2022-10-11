#include "include_all.h"






//�绰�����FLASH���ݳ�ʼ��
//
void phbk_data_flash_init(void)
{
	phbk_count_struct  init_cnt;

	memset(((uchar*)(&init_cnt)), '\x0', (sizeof(phbk_count_struct)));
	flash25_program_auto_save(phbk_start_addr_count, ((uchar*)(&init_cnt)), (sizeof(phbk_count_struct)));
}

void call_record_data_flash_init(void)
{

	_memset(((u8*)(&call_record)), '\x0', (sizeof(call_record)));

	feed_wdt;
	flash25_program_auto_save(flash_call_out_record_addr, ((uchar*)(&call_record)), (sizeof(call_record)));
	flash25_program_auto_save(flash_received_call_record_addr, ((uchar*)(&call_record)), (sizeof(call_record)));

    feed_wdt;
	flash25_program_auto_save(flash_missed_call_record_addr, ((uchar*)(&call_record)), (sizeof(call_record)));

}


/*
//�Ե绰������ҵ绰��
bool phbk_search_num(phbk_count_struct* phbk_cnt, phbk_num_struct* phbk_num, u16* posi)
{
	u32  addr;

	addr = phbk_start_addr_num_index;
	return  search_half(true, (&addr), (phbk_cnt->total), phbk_num, posi, compare_phbk_index_to_content);
}

//���������ҵ绰��
bool phbk_search_name(phbk_count_struct* phbk_cnt, phbk_name_struct* phbk_name, u16* posi)
{
	u32  addr;

	addr = phbk_start_addr_name_index;
	return	search_half(true, (&addr), (phbk_cnt->total), phbk_name, posi, compare_phbk_index_to_content);
}
*/



//����һ���绰����¼
static void phbk_add_data(phbk_count_struct* phbk_cnt, phbk_data_struct* phbk_data)
{
	flash25_program_auto_save(phbk_data_abs_addr(phbk_cnt->posi), ((uchar*)phbk_data), (sizeof(phbk_data_struct)));
}

//����һ���绰��������
static void phbk_add_num_index(phbk_count_struct* phbk_cnt, phbk_num_struct* phbk_num)
{
	phbk_data_struct  phbk_data;
	phbk_index  num_index;
	u16  i;

	spi_flash_read(((u8*)(&num_index)), phbk_start_addr_num_index, (sizeof(phbk_index)));

	for (i=0x00; i<(phbk_cnt->total); i++)
	{
		spi_flash_read(((u8*)(&phbk_data)),phbk_data_abs_addr(*(num_index+i)),  (sizeof(phbk_data_struct)));

#if 1
		if (compare_buff((phbk_data.num.buff), (phbk_data.num.lgth), (phbk_num->buff), (phbk_num->lgth)) == 1)
		{
			break;
		}
#endif
	}

	array_move((num_index+i), ((phbk_cnt->total)-i), true, array_uint_move_right);
	(*(num_index+i)) = (phbk_cnt->posi);

	flash25_program_auto_save(phbk_start_addr_num_index, ((uchar*)(&num_index)), (sizeof(phbk_index)));
}

//����һ����������
static void phbk_add_name_index(phbk_count_struct* phbk_cnt, phbk_name_struct* phbk_name)
{
	phbk_data_struct  phbk_data;
	phbk_index	num_index;
	u16  i;

	spi_flash_read(((u8*)(&num_index)), phbk_start_addr_name_index, (sizeof(phbk_index)));

	for (i=0x00; i<(phbk_cnt->total); i++)
	{
		spi_flash_read(((u8*)(&phbk_data)),phbk_data_abs_addr(*(num_index+i)),  (sizeof(phbk_data_struct)));

#if 1
		if (compare_buff((phbk_data.name.buff), (phbk_data.name.lgth), (phbk_name->buff), (phbk_name->lgth)) == 1)
		{
			break;
		}
#endif
	}


	array_move((num_index+i), ((phbk_cnt->total)-i), true, array_uint_move_right);
	(*(num_index+i)) = (phbk_cnt->posi);

	flash25_program_auto_save(phbk_start_addr_name_index, ((uchar*)(&num_index)), (sizeof(phbk_index)));
}




//�绰����������
//����type��ʾ��������     /TRUEΪ׷�Ӳ���    /FALSEΪ�޸Ĳ���
//����phbk_cntΪ����ָ��
//����phbk_dataΪһ���绰������ָ��
//
void phbk_operate_hdl(bool type, phbk_count_struct* phbk_cnt, phbk_data_struct* phbk_data)
{
	if (type) //׷�Ӳ���
	{
		phbk_add_data(phbk_cnt, phbk_data);
		phbk_add_num_index(phbk_cnt, (&(phbk_data->num)));
		phbk_add_name_index(phbk_cnt, (&(phbk_data->name)));

		(phbk_cnt->posi) ++;
		(phbk_cnt->total) ++;

		(phbk_cnt->posi) = (phbk_cnt->posi)%phone_book_total_max;

		if ((phbk_cnt->total) > phone_book_total_max)
		{
			(phbk_cnt->total) = phone_book_total_max;
		}
	}
	else      //�޸Ĳ���
	{
		;
	}
}




//�����绰��
//����d_buffΪ����ʾ�Ļ�����
//����lgthΪ�洢�����е�����������
//����posiΪ��ʵλ��
//����cntΪ�����������
//
void phbk_dis_build(u8** d_buff, u16 lgth, u16 posi, u8 cnt)
{
	phbk_data_struct  phbk_data;
	phbk_index        phbk_name_index;

	u8  i, flag;
	u8  n;

	spi_flash_read( ((u8*)(&phbk_name_index)), phbk_start_addr_name_index,(sizeof(phbk_index)));

	for (i=0x00; i<cnt; i++)
	{
		if ((posi+i) >= lgth) //������Χ
		{
			return;
		}

		spi_flash_read(((u8*)(&phbk_data)), phbk_data_abs_addr(*(phbk_name_index+(posi+i))), (sizeof(phbk_data_struct)));

		data_mask_ascii((*(d_buff+i)+0), (posi+i+1), 4);
		*(*(d_buff+i)+4) = '.';

		*(*(d_buff+i)+14) = '-';
		*(*(d_buff+i)+15) = '>';
		*(*(d_buff+i)+16) = '\x0';

		n = (phbk_data.name.lgth);  //���������ж�
		if (n > phone_book_index_short_name_max_lgth)
		{
			n = phone_book_index_short_name_max_lgth;
			flag = 1;
		}
		else
		{
			flag = 0x00;
		}

		_memcpy((*(d_buff+i)+5), (phbk_data.name.buff), n);  //������������
		_memset( (*(d_buff+i)+5+n), ' ', (9-n));  //��ʾ�հ׵�����

		if (flag) //�����ȳ���6���ֽڣ�����ʾһ���Ǻ������ֺ���
			*(*(d_buff+i)+11) = '*';

	}
}





