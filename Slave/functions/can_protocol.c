#include "sys_conf.h"

u8 can_buf[CAN_buf_size] = {0};

/**
 *@function CAN向主机发送反馈信息
 *@param 
 *				feedback:反馈信息
 *@return 
 *				0----成功
 *				其他----失败
 */
u8 CAN_send_feedback(u8 *feedback)
{
	u8 result;
	result = Can_Send_Msg(feedback, 3, master);
	return result;
}

void clean_can_buf()
{
	int i = 0;
	for(;i<CAN_buf_size;++i)
	{
		can_buf[i] = 0;
	}
}
