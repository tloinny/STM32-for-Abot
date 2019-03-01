/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: Abot master firmware
 */
 
#include "sys_conf.h"

u8 usart_buf[usart_buf_size][14];
u16 empty_flag = usart_buf_size;	/* 空位，初始值为motion_buf_size */
u16 full_flag = 0;		/* 满位，初始值为0 */
int product_count = 0;	/* 生产者计数 */
int consum_count = 0;	/* 消费者计数 */

int main(void)
{
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	DEBUG_USARTx_DMA_Config();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_Normal);
	LED0 = 1;
	delay_ms(1000);
	CAN_Call();	/* CAN广播一次，查看总线上有哪些节点 */
	delay_ms(1000);
	home_all();	/* 命令所有关节寻找原点 */
	
	while(1)
		{
			if(DEBUG_Receive_length > 0) /* 接收完一帧数据,进行数据分发 */
			{
				if(DEBUG_Rx_Buff[0]==255 && DEBUG_Rx_Buff[DEBUG_Receive_length-1]==255 && DEBUG_Receive_length == 14)
				{
					if(slave_buf_available == 1 && empty_flag == usart_buf_size && full_flag == 0)	/* 如果从机的缓存区可用，而且主机缓存区内已经没有数据，则可以直接进行数据分发 */
					{
						CAN_distribute(DEBUG_Rx_Buff,DEBUG_Receive_length);
					}else if(slave_buf_available == 0 && empty_flag != 0 && full_flag <usart_buf_size)	/* 如果从机的缓存区不可用，但是主机缓存区可用，则需要开启主机缓存功能 */
					{
						int i;
						for(i=0;i<DEBUG_Receive_length;++i)
						{
							usart_buf[product_count][i] = DEBUG_Rx_Buff[i];
						}
						(product_count == usart_buf_size)?(product_count = 0):(++product_count);	/* 生产者计数 */
						--empty_flag;	/* 获取一个空位 */
						++full_flag;	/* 释放一个满位 */
					}
				}				
				DEBUG_Receive_length = 0;
				DEBUG_RX_Start;	/* 开启下一次接收 */
			}
			
			if(slave_buf_available == 1 && empty_flag != usart_buf_size && full_flag != 0 )
			{
				CAN_distribute(usart_buf[consum_count],14);
				
				(consum_count == usart_buf_size)?(consum_count = 1):(++consum_count);	/* 消费者计数 */
							
				++empty_flag;	/* 释放一个空位 */
				--full_flag;	/* 获取一个满位 */				
			}
			
			/* CPU挂起，等待总线上的数据 */
			DelayForRespond
			
			/* 如果收到来自从机的返回，则进行反馈匹配 */
			if(Can_Receive_Msg(can_rec_buf) != 0)
			{
				match_feedback(can_rec_buf);				
			}
		}
}

