#include "include_all.h"






const u8 *sms_lib_dis[]=
{
	"已读",
	"未读",
};







//信息索引初始化
//参数ptr为信息索引
//参数base_addr为当前操作的信息存储基地址
//参数single_lgth为当前操作的信息单条信息长度
//
static void sms_index_init(sms_index_struct* ptr, u32 base_addr, u16 single_lgth)
{
	u8  i;
	u32  temp;

	ptr->total = 0x00;  //总数清零
	temp = base_addr+sms_index_content_lgth;

	for (i=0x00; i<sms_total_max; i++)
	{
		((ptr->use_infor)+i)->addr = temp;
		temp += single_lgth;
	}
}




//信息相关FLASH数据初始化
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




//搜索信息在表格中的位置
//参数ptr为表格指针
//参数type为属性、ID号、类型等特征值
//参数cnt为当前表中存在的有效地址的总数
//函数返回值    返回所要查找的特征值在表格中的位置，范围0xff表示未找到
//
static u8 sms_search(use_infor_struct* ptr, u16 type, u8 cnt)
{
	u8  i;

	if (cnt > sms_total_max) //超过范围，不予处理
		return  0xff;

	if (cnt == 0x00)         //空间空，不予处理
		return  0xff;

	for (i=0x00; i<cnt; i++)
	{
		if (((ptr+i)->type_id_serial_num) == type) //找到特征值
		{
			return  i;
		}
	}

	return  0xff;
}

//添加一条信息
//参数ptr为表格指针
//参数type为属性、ID号、类型等特征值
//参数dat_lgth为待存入的信息长度
//参数dat_buff为待存入的信息
//
static void sms_add(use_infor_struct* ptr, u16 type, u16 dat_lgth, u8* dat_buff)
{
	u32  temp;  //临时地址缓存

	temp = (ptr+(sms_total_max-1))->addr;  //取出表格中最后一个元素中的地址
	array_move(ptr, (sms_total_max-1), true, array_sms_use_move_right);  //右移

	(ptr+0)->read_flag = true;  //新存入的值始终是未读状态
	(ptr+0)->type_id_serial_num = type;
	(ptr+0)->addr = temp;

	flash25_program_auto_save(temp, dat_buff, dat_lgth);  //存入信息内容
}

//删除一条信息
//参数ptr为表格指针
//参数type为属性、ID号、类型等特征值
//参数cnt为当前表中存在的有效地址的总数
//函数返回值    删除结果    TRUE表示删除成功    FALSE表示删除失败
//
static bool sms_del(use_infor_struct* ptr, u16 type, u8 cnt)
{
	u8  i;
	u32  temp;  //临时地址缓存

	i = sms_search(ptr, type, cnt);
	if (i == 0xff) //不存在特征值
	{
		return false;
	}

	temp = (ptr+i)->addr;    //提取待删除的数据
	array_move((ptr+i+1), (cnt-1-i), false, array_sms_use_move_left);  //左移
	(ptr+(cnt-1))->addr = temp;  //删除掉的地址加入表格中排队

	return true;
}

//修改原有的信息
//参数ptr为表格指针
//参数type为属性、ID号、类型等特征值
//参数cnt为当前表中存在的有效地址的总数
//参数dat_lgth为待存入的信息长度
//参数dat_buff为待存入的信息
//函数返回值    删除结果    TRUE表示删除成功    FALSE表示删除失败
//
static bool sms_change(use_infor_struct* ptr, u16 type, u8 cnt, u16 dat_lgth, u8* dat_buff)
{
	u8  i;

	i = sms_search(ptr, type, cnt);
	if (i == 0xff) //不存在特征值
	{
		return false;
	}

	((ptr+i)->read_flag) = true;  //新修改的信息设置为未读
	flash25_program_auto_save(((ptr+i)->addr), dat_buff, dat_lgth);  //存入信息内容

	return true;
}




//信息操作处理函数  包含增加、修改、删除单条信息的操作
//参数ptr为信息索引
//参数op为当前的操作类型，见定义
//参数type为属性、ID号、类型等信息
//参数ct_lgth为信息体内容的长度，单位字节
//参数ct_ptr为信息体内容转化为字符串之后的指针
//函数返回值    操作结果    TRUE表示操作成功    FALSE表示操作失败
//
bool sms_operate_hdl(sms_index_struct* ptr, sms_op_enum op, u16 type, u16 ct_lgth, u8* ct_ptr)
{
	switch (op)
	{
	case sms_op_add:  //增加
		sms_add((ptr->use_infor), type, ct_lgth, ct_ptr);
		(ptr->total) += 1;
		break;

	case sms_op_del:  //删除
		if (sms_del((ptr->use_infor), type, (ptr->total)) == false)
		{
			return  false;
		}

		(ptr->total) -= 1;
		break;

	case sms_op_change:  //修改
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








