#ifndef  __phone_book_h__
#define  __phone_book_h__




//**************************************************************//
//**************************************************************//
//-------------      �绰��flash�洢�ṹ˵��       -------------//
//
//������Ϣ+�����ֽ��������Ա�����+�Ե绰���뽨�������Ա�����+�绰������
//
//**************************************************************//
//**************************************************************//


//��ʼ��ַ����
#define    phbk_start_addr_count        (flash_phone_book_addr)
#define    phbk_start_addr_name_index   (flash_phone_book_addr+(sizeof(phbk_count_struct)))
#define    phbk_start_addr_num_index    (flash_phone_book_addr+(sizeof(phbk_count_struct))+phone_book_index_content_lgth)

//���Ա�������ַ
//posiΪƫ����
//#define    phbk_name_index_addr(posi)   (flash_phone_book_addr+(sizeof(phbk_count_struct))+(posi*(sizeof(u16))))
//#define    phbk_num_index_addr(posi)    (flash_phone_book_addr+(sizeof(phbk_count_struct))+phone_book_index_content_lgth+(posi*(sizeof(u16))))

//����绰�����Ե�ַ
//posiΪƫ����
#define    phbk_data_abs_addr(posi)    (flash_phone_book_addr+(sizeof(phbk_count_struct))+(phone_book_index_content_lgth*2)+(posi*(sizeof(phbk_data_struct))))












extern void phbk_data_flash_init(void);
extern void call_record_data_flash_init(void);


extern bool phbk_search_num(phbk_count_struct* phbk_cnt, phbk_num_struct* phbk_num, u16* posi);
extern bool phbk_search_name(phbk_count_struct* phbk_cnt, phbk_name_struct* phbk_name, u16* posi);

extern void phbk_operate_hdl(bool type, phbk_count_struct* phbk_cnt, phbk_data_struct* phbk_data);

extern void phbk_dis_build(u8** d_buff, u16 lgth, u16 posi, u8 cnt);







#endif




