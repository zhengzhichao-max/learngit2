typedef struct
{
	u8  u_read_flag;     //  =0 未执行u盘读取;  =1 等待“确认键按下”; =2 已经按下"确认" 置2
	u8  Data_Direction;  //  =0 平台; =1 U盘; 
	u8  state;			     //成功或失败
	u8 	cmd;			       //命令
  u16 packet;          //发送数据总包数
  u16	idx;			       //当前数据发送包数  
  u16 frame;           //一分包数据包含几个节点数据。
  u16 head;            //当前的节点头指针
  u16 tail;            //当前的节点尾指针
  u8  u_send_flag;     // =0, 表示未发送或已经收到发送应答;  =1, 表示在发送中,还未收到应答;
  u8  u8_delay;        // 发送延迟等待时间, 如果延迟时间到还未收到应答,则重新发送

} rk_6033_task_t;

