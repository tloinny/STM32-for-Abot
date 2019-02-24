#include "sys_conf.h"

u8 can_buf[CAN_buf_size] = {0};
u8 can_rec_buf[CAN_buf_size] = {0};
u32 slave[slave_num_max] = {slave_0,slave_1,slave_2,slave_3,slave_4,slave_5};

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
	*(can_buf) = (u8)(rad_temp%254);
	*(can_buf + 1) = (u8)(rad_temp/254);
	*(can_buf + 3) = 0;	/* '/0'作为分隔符 */
	*(can_buf + 4) = (u8)(speed_temp%254);
	*(can_buf + 5) = (u8)(speed_temp/254);
	result = Can_Send_Msg(can_buf, CAN_buf_size, ID);
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
	for(i = 0; i < CAN_buf_size; ++i) *(can_buf + i) = cmd;
	result = Can_Send_Msg(can_buf, CAN_buf_size, ID);
	clean_can_buf();
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
		*can_buf=*(buf+i);
		*(can_buf+1)=*(buf+i+1);
		*(can_buf+3)=0;
		*(can_buf+4)=*(buf+i+2);	/* 根据buf配置can_buf的必要位 */
		if(slave[(i-1)/3] != 0 && *can_buf != 0)	/* 如果节点存在而且数据有意义，则分发数据 */
		{
			result += Can_Send_Msg(can_buf, CAN_buf_size, slave[(i-1)/3]);	/* 分发数据 */
			DEBUG_USART_DMA_Tx_Start(can_buf, CAN_buf_size);	/* 给上位机反馈 */
		}
		delay_ms(5);
		clean_can_buf();
	}
	return result;
}

void CAN_Call()
{
	u32 time_out = 10;
	u8 temp_buf[6]={0};
	u32 count = 0;
	int i = 0;
	for(;i<slave_num_max;++i)
	{
		CAN_send_cmd(C_CALL,slave[i]);
		for(;(Can_Receive_Msg(temp_buf) == 0) && count < time_out; ++count,delay_ms(6));
		if(count >= time_out || !(temp_buf[0]== 'R'&&temp_buf[1]== 'C')) /* 如果等待超时或者反馈出错则认为该节点不存在 */
		{
			slave[i] = 0;
		}
	}
}

void clean_can_buf()
{
	int i = 0;
	for(;i<CAN_buf_size;++i)
	{
		can_buf[i] = 0;
	}
}
