/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@date on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: Abot slave firmware
 */
 
#include "sys_conf.h"

/* 电机运动信息的读写过程使用了“生产者消费者模式” */
/* 以下这段声明的都是生产者消费者模式需要使用到的量 */
motion_info motion_buf[motion_buf_size];	/* 电机运动信息缓存区 */
u8 motion_buf_full = 0;
u16 empty_flag = motion_buf_size;	/* 空位，初始值为motion_buf_size */
u16 full_flag = 0;		/* 满位，初始值为0 */
int product_count = 1;	/* motion_info 生产者计数 */
int consum_count = 1;	/* motion_info 消费者计数 */
float delta_rad = 0;

u8 key = 0;	/* CAN接收返回标志位 */

int main(void)
{
	/* 初始化结构体数组的第一位 */
	motion_buf[0].rad = 0;
	motion_buf[0].dir = 0;
	motion_buf[0].speed_max = 0;
	
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	uart_init(115200);
	LED_Init();
	LED0 = 0;
	motor_init(DMA1_Channel6, (u32)&TIM3->ARR, (u32)send_buf, send_buf_size,arr_init, psc_init);	/* 初始化电机 */
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_Normal);	/* 初始化CAN总线 */
	EXTIX_Init();	/* 初始化外部中断 */
		while(1)
		{
			DelayForRespond	/* CPU挂起，等待主机的回复 */
			key = Can_Receive_Msg(can_rec_buf);		/* 接受CAN总线信息 */
			if(key != 0 && *(can_rec_buf+3) == 0 && empty_flag != 0 && full_flag < motion_buf_size)	/* 如果接收到的信息不是命令信息，而且满足生产者的生产条件 */
			{
				/* 生产者行为 */
				
				motion_buf[product_count].rad = *(can_rec_buf+1)*254+*(can_rec_buf);
				(motion_buf[product_count].rad-motion_buf[product_count - 1].rad)>0 ? (motion_buf[product_count].dir = 1) : (motion_buf[product_count].dir = 0);	/* 判断运动方向 */
				motion_buf[product_count].speed_max = *(can_rec_buf+5)*254+*(can_rec_buf+4); /* 计算运动速度 */
				motion_buf[product_count].state = *(can_rec_buf+7);
				
				(product_count == motion_buf_size)?(product_count = 1):(++product_count);	/* 生产者计数 */
				
				--empty_flag;	/* 获取一个空位 */
				++full_flag;	/* 释放一个满位 */
				CAN_send_feedback(c_motion_request);	/* 已经接收到一则运动消息，并放入缓存区，向主机发送一次运动请求 */
			}else	if(key != 0 && *(can_rec_buf+3) != 0)/* 如果接收到的信息是命令信息 */
			{
				switch(*(can_rec_buf+3))	/* 匹配命令 */
				{
					case C_READY:	/* READY命令：预先配置好send_buf，等待ACTION命令 */
						if(empty_flag < motion_buf_size && full_flag != 0 && MotorStatus() != m_moving && zeroed != 0)	/* 在运动信息缓存区可用而且电机不在运动状态时才能配置send_buf */
						{ 
							/* 消费者行为 */
							
							/* 计算与上一个位置的delta值，用于配置电机运动参数 */
							delta_rad = fabs((motion_buf[consum_count].rad - motion_buf[consum_count-1].rad)/1000);
							
							#if SLAVE0||SLAVE1||SLAVE2
							motor_point_movement_ready(motor_type*Micro_Step*ratio*(delta_rad/pi/2), motion_buf[consum_count].dir, motor_type*Micro_Step*ratio*(delta_rad/pi/2)*0.003*pi, 0.1*pi, 0.5, send_buf);
							#endif
							
							#if SLAVE3
							motor_point_movement_ready(motor_type*Micro_Step*ratio*(delta_rad/pi/2), motion_buf[consum_count].dir, motor_type*Micro_Step*ratio*(delta_rad/pi/2)*0.006*pi, 1*pi, 2, send_buf);
							#endif
							/* 用完清零上一位的数据 */
							if(consum_count - 1 > 0)
							{
								motion_buf[consum_count - 1].rad = 0;	
								motion_buf[consum_count - 1].dir = 0;
								motion_buf[consum_count - 1].speed_max = 0;	
							}
							
							(consum_count == motion_buf_size)?(consum_count = 1):(++consum_count);	/* 消费者计数 */
							
							++empty_flag;	/* 释放一个空位 */
							--full_flag;	/* 获取一个满位 */
						}
						CAN_send_feedback(c_motor_ready);	/* 通知主机已经完成一次准备工作，可以接收ACTION命令了 */
						break;
					case C_ACTION:	/* ACTION命令：开启DMA和定时器，电机根据send_buf的内容运行 */
						if(motor_run()==1)
						{
							CAN_send_feedback(c_motor_action);	/* 通知主机已经开始一次ACTION */
						}
						break;
					case C_STOP:	/* STOP命令：电机立刻停止运动 */
						motor_stop();
						CAN_send_feedback(c_motor_stop);	/* 通知主机电机已经停止 */
						break;
					case C_HOME:	/* HOME命令：电机复位 */
						motor_home();
						CAN_send_feedback(c_motor_home);	/* 通知主机电机已经到达到原点 */
						break;
					case C_MOTOR_DISABLE:	/* DISABLE命令：电机失能 */
						motor_disable();
						CAN_send_feedback(c_motor_disable);	/* 通知主机电机已经失能 */
						break;
					case C_MOTOR_ENABLE:	/* ENABLE命令：电机使能 */
						motor_enable();
						CAN_send_feedback(c_motor_enable);	/* 通知主机电机已经使能 */
						break;
					case C_CALL:					/* CALL命令：来自主机的呼叫 */
						CAN_send_feedback(c_receive_call);	/* 响应主机呼叫，通知主机本节点存在 */
						break;
				}
			}else if(key == 0 && empty_flag < motion_buf_size && full_flag != 0 && MotorStatus() != m_moving)	/* 如果主机没有发送消息或者命令，但是从机的缓存区中仍有运动信息尚未执行，则向主机发送运动请求 */
			{
				CAN_send_feedback(c_motion_request);
			}
			if(empty_flag == 0 && motion_buf_full == 0)	/* 如果缓存区没有空位，则认为motion_buf已满，通知主机不要再发，将信息暂存在主机内存中 */
			{
				CAN_send_feedback(c_buf_full);
				motion_buf_full = 1;
			}else if(empty_flag > 50 && motion_buf_full == 1)	/* 如果缓存区有大于50个空位，则认为motion_buf可用，通知主机可以继续发送 */
			{
				CAN_send_feedback(c_buf_usefull);
				motion_buf_full = 0;				
			}
			MotorStatus();	/* 尝试更新电机状态 */
		}
}

