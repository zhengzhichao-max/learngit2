/*
#ifndef  __hard_define_h__
#define  __hard_define_h__
*/
#ifndef _HARD_DEFINE_TR9_H_
#define _HARD_DEFINE_TR9_H_

#define  pin_set(pt, p_n)    (pt->BSRR = bit_move(p_n))    //��λ�˿�
#define  pin_clr(pt, p_n)    (pt->BRR = bit_move(p_n))     //��λ�˿�
#define  pin_read(pt, p_n)   ((pt->IDR & bit_move(p_n))>>p_n)    //��ȡ�˿�״̬


//--------------------------------------------------------------
//------------              (�˿ڶ���)              ------------
//--------------------------------------------------------------
#define     key_port        		(GPIOD)   		//�������˿�

#define     lcd_scl_port   	(GPIOE)   	//ʱ��
#define     lcd_cs_port    	(GPIOE)   	//Ƭѡ
#define     lcd_rst_port   	(GPIOE)   	//��λ
#define     lcd_a0_port    	(GPIOE)   	//����/����ѡ��
#define     lcd_io_port    	(GPIOE)   	//����
#define     lcd_bl_port    	(GPIOB)   	//����




#define    pow_ctrl_print_port    (GPIOB)//(11)   //��ӡ����Դ���ƶ˿�

#define    print_mtnb_port   (GPIOE)	//(9)    //��ӡ������
#define    print_mtb_port    (GPIOE)	//(10)   //��ӡ������
#define    print_mtna_port   (GPIOE)	//(11)   //��ӡ������
#define    print_mta_port    (GPIOE)	//(12)   //��ӡ������
#define    print_di_port     (GPIOA)	//(13)   //��ӡ������
#define    print_clk_port    (GPIOA)	//(14)   //��ӡ������
#define    print_lat_port    (GPIOE)	//(15)   //��ӡ������

#define    print_hot_port1    (GPIOA)	//(2)  //��ӡ�����ȿ���
#define    print_hot_port2    (GPIOA)	//(15)  //��ӡ�����ȿ���


//----------已修�?---------
#define     key_up_pin     	((u8)3)
#define     key_down_pin    ((u8)4)
#define     key_menu_pin    ((u8)5)
#define     key_print_pin   ((u8)6)
#define     key_ok_pin    	((u8)7)


#define     lcd_scl_pin    ((u8)6)
#define     lcd_cs_pin     ((u8)3)
#define     lcd_rst_pin    ((u8)4)
#define     lcd_a0_pin     ((u8)5)
#define     lcd_io_pin     ((u8)7)
#define     lcd_bl_pin     ((u8)15)

#define    ic_card_in_port   (GPIOB)	//(4)  //IC��������˿�
#define    ic_card_rst_port  (GPIOB)	//(5)  //IC����λ���ƶ˿�
#define    ic_card_clk_port  (GPIOD)	//(6)  //IC��ʱ�ӿ��ƶ˿�
#define    ic_card_io_port   (GPIOD)	//(7)  //IC�����ݿ��ƶ˿�

#define    ic_card_in_pin   ((uchar)0)//(GPIOB)//(4)  //IC��������˿�
#define    ic_card_rst_pin  ((uchar)14)//(GPIOB)//(5)  //IC����λ���ƶ˿�
#define    ic_card_clk_pin  ((uchar)9)//(GPIOB)//(6)  //IC��ʱ�ӿ��ƶ˿�
#define    ic_card_io_pin   ((uchar)10)//(GPIOB)//(7)  //IC�����ݿ��ƶ˿�


#define    pow_ctrl_print_pin    ((uchar)2)//(GPIOA)//(15)   //��ӡ����Դ���ƶ˿�

#define    print_mtb_pin    ((uchar)9)//(GPIOD)//(12)   //��ӡ������
#define    print_mtnb_pin   ((uchar)8)//(GPIOD)//(11)   //��ӡ������
#define    print_mtna_pin   ((uchar)11)//(GPIOD)//(9)    //��ӡ������
#define    print_mta_pin    ((uchar)10)//(GPIOD)//(10)   //��ӡ������

#define    print_di_pin     ((uchar)6)//(GPIOD)//(13)   //��ӡ������
#define    print_clk_pin    ((uchar)5)//(GPIOD)//(14)   //��ӡ������
#define    print_lat_pin    ((uchar)2)//(GPIOD)//(15)   //��ӡ������     xh100

//#define    print_hot_pin    ((uchar))//(GPIOC)//(1-6)  //��ӡ�����ȿ���
#define    print_hot_1pin    ((uchar)4)//(GPIOC)//(1-6)  //��ӡ�����ȿ���
#define    print_hot_2pin    ((uchar)4)//(GPIOC)//(1-6)  //��ӡ�����ȿ���
#define    print_hot_3pin    ((uchar)4)//(GPIOC)//(1-6)  //��ӡ�����ȿ���
#define    print_hot_4pin    ((uchar)7)//(GPIOC)//(1-6)  //��ӡ�����ȿ���
#define    print_hot_5pin    ((uchar)7)//(GPIOC)//(1-6)  //��ӡ�����ȿ���
#define    print_hot_6pin    ((uchar)7)//(GPIOC)//(1-6)  //��ӡ�����ȿ���


#define     lcd_scl_H    (lcd_scl_port->BSRR = bit_move(lcd_scl_pin))
#define     lcd_cs_H     (lcd_cs_port->BSRR  = bit_move(lcd_cs_pin))
#define     lcd_rst_H    (lcd_rst_port->BSRR = bit_move(lcd_rst_pin))
#define     lcd_a0_H     (lcd_a0_port->BSRR  = bit_move(lcd_a0_pin))
#define     lcd_io_H     (lcd_io_port->BSRR  = bit_move(lcd_io_pin))

#define     lcd_scl_L    (lcd_scl_port->BRR = bit_move(lcd_scl_pin))
#define     lcd_cs_L     (lcd_cs_port->BRR  = bit_move(lcd_cs_pin))
#define     lcd_rst_L    (lcd_rst_port->BRR = bit_move(lcd_rst_pin))
#define     lcd_a0_L     (lcd_a0_port->BRR  = bit_move(lcd_a0_pin))
#define     lcd_io_L     (lcd_io_port->BRR  = bit_move(lcd_io_pin))

#define     lcd_bl_off    (lcd_bl_port->BRR  = bit_move(lcd_bl_pin))     //�򿪱���
#define     lcd_bl_on   (lcd_bl_port->BSRR = bit_move(lcd_bl_pin))     //�رձ���

//key5:key4:key3:key2:key1 >>>>  PE5:PE4:PE3:PE2:PE0   PD-89101112
#define     key_read_data()  ((key_port->IDR & 0x0078)>>3)

#define    ic_insert_read()    pin_read(ic_card_in_port, ic_card_in_pin)   //��ȡIC���Ĳ���״̬
#define    ic_card_dat_read()  pin_read(ic_card_io_port, ic_card_io_pin)   //��ȡIC�����ݽӿ�״̬
#define    ic_card_clk_read()  pin_read(ic_card_clk_port, ic_card_clk_pin) //��ȡIC��ʱ�ӽӿ�״̬

#define    ic_card_rst_high    pin_set(ic_card_rst_port, ic_card_rst_pin)
#define    ic_card_rst_low     pin_clr(ic_card_rst_port, ic_card_rst_pin)
#define    ic_card_clk_high    pin_set(ic_card_clk_port, ic_card_clk_pin)
#define    ic_card_clk_low     pin_clr(ic_card_clk_port, ic_card_clk_pin)
#define    ic_card_io_high     pin_set(ic_card_io_port, ic_card_io_pin)
#define    ic_card_io_low      pin_clr(ic_card_io_port, ic_card_io_pin)


#define    pow_print_on    pin_set(pow_ctrl_print_port, pow_ctrl_print_pin)    //�򿪴�ӡ����Դ
#define    pow_print_off   pin_clr(pow_ctrl_print_port, pow_ctrl_print_pin)    //�رմ�ӡ����Դ

#define    print_mtnb_high    pin_set(print_mtnb_port, print_mtnb_pin)//GPIOE_8
#define    print_mtnb_low     pin_clr(print_mtnb_port, print_mtnb_pin)
#define    print_mtb_high     pin_set(print_mtb_port, print_mtb_pin)//GPIOE_9
#define    print_mtb_low      pin_clr(print_mtb_port, print_mtb_pin)
#define    print_mtna_high    pin_set(print_mtna_port, print_mtna_pin)//GPIOE_11
#define    print_mtna_low     pin_clr(print_mtna_port, print_mtna_pin)
#define    print_mta_high    pin_set(print_mta_port, print_mta_pin)//GPIOE_10
#define    print_mta_low     pin_clr(print_mta_port, print_mta_pin)
#define    print_di_high     pin_set(print_di_port, print_di_pin)//GPIOA_6
#define    print_di_low      pin_clr(print_di_port, print_di_pin)
#define    print_clk_high    pin_set(print_clk_port, print_clk_pin)//GPIOA_5
#define    print_clk_low     pin_clr(print_clk_port, print_clk_pin)
#define    print_lat_high    pin_set(print_lat_port, print_lat_pin)//GPIOE_2
#define    print_lat_low     pin_clr(print_lat_port, print_lat_pin)



//--------------------------------------------------------------
//--------------------------------------------------------------
//--------------------------------------------------------------
#endif

