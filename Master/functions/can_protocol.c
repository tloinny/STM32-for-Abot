/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: Abot关节节点CAN通讯协议驱动
 */
 
#include "sys_conf.h"

u8 can_send_buf[can_buf_size] = {0};	/* CAN总线发送缓存区 */
u8 can_rec_buf[can_buf_size] = {0};	/* CAN总线接收缓存区 */
u32 slave[slave_num_max] = {slave_0,slave_1,slave_2,slave_3,slave_4,slave_5};	/* 节点ID表 */
u8 slave_num = 0;	/* 可用节点计数 */
u8 ready_num = 0;	/* 已准备好的节点计数 */
u8 ready_list[slave_num_max] = {0};	/* 准备好的节点列表 */
u8 arrive_num = 0;	/* 到达的节点计数 */
u8 arrive_list[slave_num_max] = {0};	/* 到达的节点列表 */
u8 slave_buf_available = 1;	/* 表示从机缓存区是否可用，用于决定是否开启主机缓存功能 */

/**
 *@function CAN向从机发送速度信息和弧度制的角度信息
 *@param 
 *			rad:角度信息
 *			speed:速度信息	
 *				ID:从机地址	如:slave_0|slave_1,表示发送给从机0和从机1
 *@return 
 *				0----成功
 *				其他----失败
 */
u8 CAN_send_motion_info(float rad, float speed, u32 ID)
{
	u8 result = 0;
	u16 rad_temp = (u16)(rad * 1000);	/* 扩大1000倍，传输后精确到0.001 */
	u16 speed_temp = (u16)(speed * 10);
	*(can_send_buf) = (u8)(rad_temp%254);
	*(can_send_buf + 1) = (u8)(rad_temp/254);
	*(can_send_buf + 3) = 0;	/* '/0'作为分隔符 */
	*(can_send_buf + 4) = (u8)(speed_temp%254);
	*(can_send_buf + 5) = (u8)(speed_temp/254);
	result = Can_Send_Msg(can_send_buf, can_buf_size, ID);
	return result;
}

/**
 *@function CAN向从机发送指令
 *@param 
 *				cmd:命令
 *				ID:从机地址	如:slave_0|slave_1,表示发送给从机0和从机1
 *@return 
 *				0----成功
 *				其他----失败
 */
u8 CAN_send_cmd(u8 cmd, u32 ID)
{
	u16 i;
	u8 result;
	for(i = 0; i < can_buf_size; ++i) *(can_send_buf + i) = cmd;
	result = Can_Send_Msg(can_send_buf, can_buf_size, ID);
	clean_can_send_buf();
	return result;
}

/**
 *@function CAN向从机进行数据分发
 *@param 
 *				buf:数据源
 *				len:数据源长度
 *@return 
 *				0----成功
 *				其他----失败
 */
u8 CAN_distribute(u8 * buf, u8 len)
{
	int i = 1;
	u8 result=0;
	for(;i<len-2;i+=3)	/* 以增量为3遍历buf */
	{
		*can_send_buf=*(buf+i);
		*(can_send_buf+1)=*(buf+i+1);
		*(can_send_buf+3)=0;
		*(can_send_buf+4)=*(buf+i+2);	/* 根据buf配置can_send_buf的必要位 */
		if(slave[(i-1)/3] != 0)	/* 如果节点存在，则分发数据 */
		{
			result += Can_Send_Msg(can_send_buf, can_buf_size, slave[(i-1)/3]);	/* 分发数据 */
			DEBUG_USART_DMA_Tx_Start(can_send_buf, can_buf_size);	/* 给上位机反馈 */
		}
		DelayForRespond
		clean_can_send_buf();
	}
	return result;
}

/**
 *@function CAN向从机进行呼叫，确定哪些节点是存在于总线上的，最终填写在slav[]中。不存在的节点ID置零
 *@param void
 *@return void
 */
void CAN_Call()
{
	u32 time_out = 10;
	u8 temp_buf[8]={0};
	u32 count = 0;
	int i = 0;
	for(;i<slave_num_max;++i)
	{
		CAN_send_cmd(C_CALL,slave[i]);
		for(count=0;(Can_Receive_Msg(temp_buf) == 0) && count < time_out; ++count,delay_ms(6));	/* 阻塞性等待从机回复 */
		if(count >= time_out || !(temp_buf[0]== 'R'&&temp_buf[1]== 'C')) /* 如果等待超时或者反馈出错则认为该节点不存在 */
		{
			slave[i] = 0;
		}else
		{
			++slave_num;	/* 计算可用节点数 */
		}
	}
}

/**
 *@function 清除can_send_buf中的数据
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

/**
 *@function 清除can_rec_buf中的数据
 *@param void
 *@return void
 */
void clean_can_rec_buf()
{
	int i = 0;
	for(;i<can_buf_size;++i)
	{
		can_rec_buf[i] = 0;
	}
}

/**
 *@function 使所有节点寻找关节原点
 *@param void
 *@return 
 *				1：所有原点寻找成功
 *				0：出错
 */
u8 home_all()
{
	u8 count;
	u8 i;
		int n = 0;
	u32 rec_history[slave_num_max] = {0};	/* 记录哪些节点已经发送过信息,避免重复发送的情况 */
	u8 temp_buf[8]={0};
	CAN_send_cmd(C_HOME,slave_all);
	for(count=0;count<slave_num;)	/* 阻塞性等待回复 */
	{
		DelayForRespond
		if(Can_Receive_Msg(temp_buf))
		{
			if(temp_buf[0] == 'H' && (temp_buf[1]-'0')>=0 && (temp_buf[1]-'0')<slave_num_max)
			{
				if(rec_history[(temp_buf[1]-'0')] == 0 && slave[(temp_buf[1]-'0')] != 0)	/* 防止重复 */
				{
					rec_history[(temp_buf[1]-'0')] = 1;
					++count;
				}
			}	
		}	
		for(n=0;n<can_buf_size;++n)
		{
			temp_buf[n] = 0;
		}
	}
	for(i=0;i<slave_num_max;++i)	/* 判断是否所有可用节点都已经寻找到原点 */
	{
		if(slave[i]*rec_history[i] != slave[i])
		{
			clean_can_rec_buf();
			return 0;
		}else
		{
			arrive_list[i] = rec_history[i];	/* 标志为到达指定位置 */
		}
	}
	return 1;
}

/**
 *@function 匹配来自从机的反馈信息，并且做出反应
 *@param void
 *@return void
 */
void match_feedback(u8* feedback)
{
	u8 i;
	u8 result = 0;
	switch(*feedback)
	{
		case 'Q':	/* c_motion_request:表示某个从机发来运动请求 */
				if((*(feedback+1)-'0')>=0 && (*(feedback+1)-'0')<slave_num_max)
				{
					if(arrive_list[(*(feedback+1)-'0')] == 1)	/* 只有当从机已经到达指定位置时才能进行下一次运动的准备工作 */
						CAN_send_cmd(C_READY,slave[*(feedback+1)-'0']);	/* 通知该从机做好准备工作 */
				}
			break;
		case 'R':	/* c_motor_ready:表示某个从机已经做好准备工作 */
				if((*(feedback+1)-'0')>=0 && (*(feedback+1)-'0')<slave_num_max)
				{
					if(ready_list[(*(feedback+1)-'0')] == 0)
					{
						ready_list[(*(feedback+1)-'0')] = 1;	/* 标志为已准备完成 */
						++ready_num;
					}
				}
				for(i=0;i<slave_num_max && ready_num == slave_num;++i) /* 当所有有效节点都完成准备工作 */
				{
					result = 1 && (slave[i]*ready_list[i] == slave[i]);	/* 防止一些在未知原因下进入总线的节点对此产生干扰 */
				}
				if(result) CAN_send_cmd(C_ACTION,slave_all);	/* 通知所有从机开始驱动电机 */
			break;
		case 'H':	/* c_motor_home:表示某个从机的电机已经到达原点位置 */
			break;
		case 'A':	/* c_motor_arrive或者c_motor_action */
				switch(*(feedback+1))
				{
					case 'R':	/* c_motor_arrive */
						if((*(feedback+2)-'0')>=0 && (*(feedback+2)-'0')<slave_num_max)
						{
							if(arrive_list[(*(feedback+2)-'0')] == 0)
							{
								arrive_list[(*(feedback+2)-'0')] = 1;	/* 标志为已到达指定位置 */
								++arrive_num;
							}
						}
						break;
					case 'C':	/* c_motor_action */
						if((*(feedback+2)-'0')>=0 && (*(feedback+2)-'0')<slave_num_max)
						{
							if(arrive_list[(*(feedback+2)-'0')] == 1)
							{
								arrive_list[(*(feedback+2)-'0')] = 0;	/* 标志为未到达指定位置 */
								--arrive_num;
							}
						}
						break;
					default:
						break;
				}
			break;
		case 'S':	/* c_motor_stop:表示某个从机的电机已经停止 */
			break;
		case 'D':	/* c_motor_disable:表示某个从机的电机已经失能 */								
			break;
		case 'E':	/* c_motor_ensable:表示某个从机的电机已经使能 */
			break;
		case 'F':	/* c_buf_full:表示某个从机的运动信息缓存区已满 */
			slave_buf_available = 0;
		break;
		case 'U':	/* c_buf_usefull:表示某个从机的运动信息缓存区恢复可用 */
			slave_buf_available = 1;
		break;
		default:
			break;		
	}
	clean_can_rec_buf();
}
