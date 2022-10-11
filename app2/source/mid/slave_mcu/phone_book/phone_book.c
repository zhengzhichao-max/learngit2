#include "include_all.h"






//电话本相关FLASH数据初始化
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
//以电话号码查找电话本
bool phbk_search_num(phbk_count_struct* phbk_cnt, phbk_num_struct* phbk_num, u16* posi)
{
	u32  addr;

	addr = phbk_start_addr_num_index;
	return  search_half(true, (&addr), (phbk_cnt->total), phbk_num, posi, compare_phbk_index_to_content);
}

//以姓名查找电话本
bool phbk_search_name(phbk_count_struct* phbk_cnt, phbk_name_struct* phbk_name, u16* posi)
{
	u32  addr;

	addr = phbk_start_addr_name_index;
	return	search_half(true, (&addr), (phbk_cnt->total), phbk_name, posi, compare_phbk_index_to_content);
}
*/



//增加一条电话本记录
static void phbk_add_data(phbk_count_struct* phbk_cnt, phbk_data_struct* phbk_data)
{
	flash25_program_auto_save(phbk_data_abs_addr(phbk_cnt->posi), ((uchar*)phbk_data), (sizeof(phbk_data_struct)));
}

//增加一条电话号码索引
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

//增加一条姓名索引
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




//电话本操作函数
//参数type表示操作类型     /TRUE为追加操作    /FALSE为修改操作
//参数phbk_cnt为计数指针
//参数phbk_data为一条电话本内容指针
//
void phbk_operate_hdl(bool type, phbk_count_struct* phbk_cnt, phbk_data_struct* phbk_data)
{
	if (type) //追加操作
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
	else      //修改操作
	{
		;
	}
}




//建立电话本
//参数d_buff为待显示的缓冲区
//参数lgth为存储区域中的数据总条数
//参数posi为其实位置
//参数cnt为待加载项计数
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
		if ((posi+i) >= lgth) //超过范围
		{
			return;
		}

		spi_flash_read(((u8*)(&phbk_data)), phbk_data_abs_addr(*(phbk_name_index+(posi+i))), (sizeof(phbk_data_struct)));

		data_mask_ascii((*(d_buff+i)+0), (posi+i+1), 4);
		*(*(d_buff+i)+4) = '.';

		*(*(d_buff+i)+14) = '-';
		*(*(d_buff+i)+15) = '>';
		*(*(d_buff+i)+16) = '\x0';

		n = (phbk_data.name.lgth);  //姓名长度判断
		if (n > phone_book_index_short_name_max_lgth)
		{
			n = phone_book_index_short_name_max_lgth;
			flag = 1;
		}
		else
		{
			flag = 0x00;
		}

		_memcpy((*(d_buff+i)+5), (phbk_data.name.buff), n);  //加载姓名内容
		_memset( (*(d_buff+i)+5+n), ' ', (9-n));  //显示空白的区域

		if (flag) //若长度超过6个字节，则显示一个星号在名字后面
			*(*(d_buff+i)+11) = '*';

	}
}





