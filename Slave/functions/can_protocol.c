/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: Abot关节控制节点CAN总线通讯协议驱动
 */

#include "sys_conf.h"

u8 can_send_buf[can_buf_size] = {0};	/* CAN发送缓存区 */
u8 can_rec_buf[can_buf_size] = {0};	/* CAN接收缓存区 */

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

/**
*@function 清楚CAN发送缓存区内容
 *@param void
 *@return void
 */
void clean_can_send_buf()
{
	int i = 0;
	for(;i<can_buf_size;++i)
	{
		can_send_buf[i] = 0;
	}
}
