#include "fat_user.h"
#include "include_all.h"












FATFS   fatfs;    //�ļ�ϵͳ
FIL     files;    //�ļ�
DIR     dirs;     //·��








//*****************************************************//
//*******           ��ʼ���ļ�ϵͳ              *******//
//��������ֵ������
//*****************************************************//
FRESULT fat_init(void)
{
    uchar    s;
    FRESULT  res;

    s = disk_initialize(0);
    if (s)   //��ʼ��ʧ��
    {
        return  FR_NOT_READY;
    }

    res = f_mount((&fatfs), (""), (1));    //��װ�ļ�ϵͳ
    return  res;
}




//*****************************************************//
//*******              �����ļ�                 *******//
//FATFS�ļ�ϵͳ���ļ�����ͬʱ�򿪶༶�ļ���
//
//����file_nameΪ�ļ���
//����lgthΪ�򿪳ɹ���д���ļ�����
//��������ֵ������
//*****************************************************//
FRESULT fat_search_file(const TCHAR * file_name, ulong * lgth)
{
    FRESULT  ret;

    ret = f_open((&files), file_name, (FA_OPEN_EXISTING | FA_READ));  //���ļ�
    if (ret == FR_OK)
    {
        *lgth = files.fsize; //��ȡ�ļ�����
    }
    else
    {
        *lgth = 0x00;
    }

    f_close(&files);    //�ر��ļ�
    return  ret;
}




//*****************************************************//
//*******              �����ļ�                 *******//
/*
FATFS�ļ�ϵͳ�����ļ���ͬʱ����ͬʱ�����ļ���
���Ե������ļ�����FR_NO_PATHʱ������Ҫ���ȴ����ļ���
֮����ܴ����ļ����ļ��в�����ͬʱ�༶������ֻ�ܵ�������
*/
//����file_nameΪ�ļ���
//��������ֵ������
//
//�������涨·���ָ�������Ϊ\��д����ӦΪ'\\'
//�������涨�������ļ�·������󳤶�Ϊ64���ֽ�
//�������涨�������ļ�����ļ��м���Ϊ6��
//*****************************************************//
#define    file_path_max_lgth    64
#define    file_path_max_level   6
FRESULT fat_create_file(const TCHAR * file_name)
{
    FRESULT  ret;
    uchar   i;
    bit_enum  flag;
    uchar   lgth;  //�ܳ���
    uchar   n;     //�ļ��м���
    uchar   str_cnt[file_path_max_level];
    TCHAR   buff[file_path_max_lgth + 1];

    ret = f_open((&files), file_name, (FA_CREATE_ALWAYS | FA_WRITE));  //�����ļ�
    if (ret == FR_NO_PATH) //�Ҳ���·��
    {
        lgth = strlen(file_name);  //�����ܳ���
        if (lgth > file_path_max_lgth)
        {
            return  ret;
        }

        n = 0x00;
        for (i = 0x00; i < lgth; i++)
        {
            if ((*(file_name + i)) == '\\')
            {
                if (n >= file_path_max_level) //�ļ��м��������涨ֵ
                {
                    return  ret;
                }

                str_cnt[n] = i;
                n++;
            }
        }

        flag = _false_;
        for (i = 1; i < n; i++) //��һ��Ϊ�洢�豸�������轨��
        {
            memcpy(buff, file_name, (*(str_cnt + i)));
            *(buff + (*(str_cnt + i))) = '\x0'; //������־

            if (flag == _false_) //��ѯĿ¼�ļ���
            {
                if (f_opendir((&dirs), buff) != FR_OK)
                {
                    flag = _true_;  //�����·����Ҫ���´���
                }
            }

            if (flag == _true_) //����Ŀ¼�ļ���
            {
                if (f_mkdir(buff) != FR_OK) //����Ŀ¼�ļ��д���
                {
                    return  ret;
                }
            }
        }

        ret = f_open((&files), file_name, (FA_CREATE_ALWAYS | FA_WRITE));  //���½����ļ�
    }

    f_close(&files);    //�ر��ļ�
    return  ret;
}




//*****************************************************//
//*******                ���ļ�                 *******//
//��ȡ�ļ����ݣ��������ļ���ر��ļ��Ĺ���
//
//����file_nameΪ�ļ������򿪵��ļ�������·��
//����d_buffΪ��Ŷ����ļ������ݻ�����
//����w_pΪ����ȡ���ļ��е�λ��
//����cntΪָ����ȡ�����ݵĳ���
//��������ֵ������
//*****************************************************//
FRESULT fat_read_file(const TCHAR * file_name, uchar * d_buff, ulong w_p, uint cnt)
{
    FRESULT res;

    res = f_open((&files), file_name, (FA_OPEN_EXISTING | FA_READ)); //��Ŀ¼���ļ�

    if (res == FR_OK)
    {
        res = f_lseek((&files), w_p);

        if (res == FR_OK)
        {
            res = f_read((&files), d_buff, ((UINT)cnt), ((UINT *)(&cnt))); //��ȡ�ļ�����
        }

    }

    f_close(&files);   //�������һ��Ҫ�ر��ļ���������ļ������
    return  res;
}




//*****************************************************//
//*******                д�ļ�                 *******//
//����д������д�뵽ָ�����ļ��У��������ļ���ر��ļ��Ĺ���
//
//����file_nameΪ�ļ���д����ļ�������·��
//����d_buffΪ��д�����ݻ�����
//����w_pΪ��д���ļ���Ҫд��ָ���ļ��е�λ��
//����cntΪָ��д�����ݵĳ���
//��������ֵ������
//*****************************************************//
FRESULT fat_write_file(const TCHAR * file_name, uchar * d_buff, ulong w_p, uint cnt)
{
    FRESULT res;

    res = f_open((&files), file_name, (FA_OPEN_EXISTING | FA_WRITE)); //��Ŀ¼���ļ�

    if (res == FR_OK)
    {
        res = f_lseek((&files), w_p);   //�޸�д�뵽ָ�����ļ��е�дָ��

        if (res == FR_OK)
        {
            res = f_write((&files), d_buff, ((UINT)cnt), ((UINT *)(&cnt))); //д���д�����ݵ�ָ���ļ���
        }
    }

    f_close(&files);   //�������һ��Ҫ�ر��ļ���������ļ������
    return  res;
}




