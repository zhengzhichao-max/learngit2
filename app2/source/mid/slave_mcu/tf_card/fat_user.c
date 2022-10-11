#include "fat_user.h"
#include "include_all.h"












FATFS   fatfs;    //文件系统
FIL     files;    //文件
DIR     dirs;     //路径








//*****************************************************//
//*******           初始化文件系统              *******//
//函数返回值见定义
//*****************************************************//
FRESULT fat_init(void)
{
    uchar    s;
    FRESULT  res;

    s = disk_initialize(0);
    if (s)   //初始化失败
    {
        return  FR_NOT_READY;
    }

    res = f_mount((&fatfs), (""), (1));    //安装文件系统
    return  res;
}




//*****************************************************//
//*******              搜索文件                 *******//
//FATFS文件系统打开文件可以同时打开多级文件夹
//
//参数file_name为文件名
//参数lgth为打开成功后写入文件长度
//函数返回值见定义
//*****************************************************//
FRESULT fat_search_file(const TCHAR * file_name, ulong * lgth)
{
    FRESULT  ret;

    ret = f_open((&files), file_name, (FA_OPEN_EXISTING | FA_READ));  //打开文件
    if (ret == FR_OK)
    {
        *lgth = files.fsize; //获取文件长度
    }
    else
    {
        *lgth = 0x00;
    }

    f_close(&files);    //关闭文件
    return  ret;
}




//*****************************************************//
//*******              创建文件                 *******//
/*
FATFS文件系统创建文件的同时不能同时创建文件夹
所以当创建文件返回FR_NO_PATH时，则需要首先创建文件夹
之后才能创建文件，文件夹不可以同时多级创建，只能单级创建
*/
//参数file_name为文件名
//函数返回值见定义
//
//本函数规定路径分隔符必须为\，写法上应为'\\'
//本函数规定建立的文件路径的最大长度为64个字节
//本函数规定建立的文件最大文件夹级数为6级
//*****************************************************//
#define    file_path_max_lgth    64
#define    file_path_max_level   6
FRESULT fat_create_file(const TCHAR * file_name)
{
    FRESULT  ret;
    uchar   i;
    bit_enum  flag;
    uchar   lgth;  //总长度
    uchar   n;     //文件夹级数
    uchar   str_cnt[file_path_max_level];
    TCHAR   buff[file_path_max_lgth + 1];

    ret = f_open((&files), file_name, (FA_CREATE_ALWAYS | FA_WRITE));  //建立文件
    if (ret == FR_NO_PATH) //找不到路径
    {
        lgth = strlen(file_name);  //计算总长度
        if (lgth > file_path_max_lgth)
        {
            return  ret;
        }

        n = 0x00;
        for (i = 0x00; i < lgth; i++)
        {
            if ((*(file_name + i)) == '\\')
            {
                if (n >= file_path_max_level) //文件夹级数超过规定值
                {
                    return  ret;
                }

                str_cnt[n] = i;
                n++;
            }
        }

        flag = _false_;
        for (i = 1; i < n; i++) //第一级为存储设备本身，无需建立
        {
            memcpy(buff, file_name, (*(str_cnt + i)));
            *(buff + (*(str_cnt + i))) = '\x0'; //结束标志

            if (flag == _false_) //查询目录文件夹
            {
                if (f_opendir((&dirs), buff) != FR_OK)
                {
                    flag = _true_;  //后面的路径需要重新创建
                }
            }

            if (flag == _true_) //创建目录文件夹
            {
                if (f_mkdir(buff) != FR_OK) //创建目录文件夹错误
                {
                    return  ret;
                }
            }
        }

        ret = f_open((&files), file_name, (FA_CREATE_ALWAYS | FA_WRITE));  //重新建立文件
    }

    f_close(&files);    //关闭文件
    return  ret;
}




//*****************************************************//
//*******                读文件                 *******//
//读取文件内容，包含打开文件与关闭文件的过程
//
//参数file_name为文件名待打开的文件的完整路径
//参数d_buff为存放读出文件的数据缓冲器
//参数w_p为待读取的文件中的位置
//参数cnt为指定读取的数据的长度
//函数返回值见定义
//*****************************************************//
FRESULT fat_read_file(const TCHAR * file_name, uchar * d_buff, ulong w_p, uint cnt)
{
    FRESULT res;

    res = f_open((&files), file_name, (FA_OPEN_EXISTING | FA_READ)); //打开目录项文件

    if (res == FR_OK)
    {
        res = f_lseek((&files), w_p);

        if (res == FR_OK)
        {
            res = f_read((&files), d_buff, ((UINT)cnt), ((UINT *)(&cnt))); //读取文件内容
        }

    }

    f_close(&files);   //操作完后一定要关闭文件，否则该文件会崩溃
    return  res;
}




//*****************************************************//
//*******                写文件                 *******//
//将待写的数据写入到指定的文件中，包含打开文件与关闭文件的过程
//
//参数file_name为文件名写入的文件的完整路径
//参数d_buff为待写的数据缓冲器
//参数w_p为待写的文件将要写到指定文件中的位置
//参数cnt为指定写入数据的长度
//函数返回值见定义
//*****************************************************//
FRESULT fat_write_file(const TCHAR * file_name, uchar * d_buff, ulong w_p, uint cnt)
{
    FRESULT res;

    res = f_open((&files), file_name, (FA_OPEN_EXISTING | FA_WRITE)); //打开目录项文件

    if (res == FR_OK)
    {
        res = f_lseek((&files), w_p);   //修改写入到指定到文件中的写指针

        if (res == FR_OK)
        {
            res = f_write((&files), d_buff, ((UINT)cnt), ((UINT *)(&cnt))); //写入待写的数据到指定文件中
        }
    }

    f_close(&files);   //操作完后一定要关闭文件，否则该文件会崩溃
    return  res;
}




