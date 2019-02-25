#include "sys_conf.h"

typedef struct motion_info
{
	float rad;
	u8 dir;
	float speed_max;
} motion_info;

motion_info motion_buf[motion_buf_size];	/* 电机运动参数缓存区 */
extern u16 send_buf[send_buf_size]; /* 脉冲发送缓存区 */ 

int key = 0;
int product_count = 1;	/* motion_info 生产者计数 */
int consum_count = 1;	/* motion_info 消费者计数 */
extern int current_position;
float delta_rad = 0; 
	
u8 empty_flag = motion_buf_size;	/* 空位，初始值为motion_buf_size */
u8 full_flag = 0;		/* 满位，初始值为0 */

u8 rec_buf[8] = {0};
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
	EXTIX_Init();
		while(1)
		{
			key = Can_Receive_Msg(rec_buf);		/* 接受CAN总线信息 */
			if(key != 0 && *(rec_buf+3) == 0 && empty_flag != 0 && full_flag < motion_buf_size)	/* 如果接收到的信息不是命令信息，而且满足生产者的生产条件 */
			{
				/* 生产者行为 */
				motion_buf[product_count].rad = *(rec_buf+1)*254+*(rec_buf);	/* 计算关节转角弧度值 */
				((motion_buf[product_count].rad-motion_buf[product_count - 1].rad)>0)?(motion_buf[product_count].dir = 1):(motion_buf[product_count].dir = 0);	/* 判断运动方向 */
				motion_buf[product_count].speed_max = *(rec_buf+5)*254+*(rec_buf+4); /* 计算运动速度 */
				printf("-----------\r\n");
				printf("rad:%f\r\n",motion_buf[product_count].rad);
				printf("dir:%d\r\n",motion_buf[product_count].dir);
				printf("speed:%f\r\n",motion_buf[product_count].speed_max);
				(product_count == motion_buf_size)?(product_count = 1):(++product_count);
				--empty_flag;	/* 获取一个空位 */
				++full_flag;	/* 释放一个满位 */
			}else	if(key != 0 && *(rec_buf+3) != 0)/* 如果接收到的信息是命令信息 */
			{
				switch(*(rec_buf+3))	/* 匹配命令 */
				{
					case C_READY:	/* READY命令：预先配置好send_buf，等待ACTION命令 */
						printf("recieve r \r\n");
						if(empty_flag < motion_buf_size && full_flag != 0)
						{
							/* 消费者行为 */
							
							/* 计算与上一个位置的delta值 */
							delta_rad = (motion_buf[consum_count].rad - motion_buf[consum_count-1].rad)/1000;
							motor_move_ready(motor_type*Micro_Step*ratio*delta_rad, motion_buf[consum_count].dir, motion_buf[consum_count].speed_max, pi, 5, 5, send_buf);
							/* 用完清零上一位的数据 */
							if(consum_count - 1 > 0)
							{
								motion_buf[consum_count - 1].rad = 0;	
								motion_buf[consum_count - 1].dir = 0;
								motion_buf[consum_count - 1].speed_max = 0;	
							}
							
							(consum_count == motion_buf_size)?(consum_count = 1):(++consum_count);
							++empty_flag;	/* 释放一个空位 */
							--full_flag;	/* 获取一个满位 */
						}
						CAN_send_feedback(c_motor_ready);	/* 通知主机已经完成一次准备工作，可以接收ACTION命令了 */
						break;
					case C_ACTION:	/* ACTION命令：开启DMA和定时器，电机立刻根据send_buf的内容运行 */
						printf("recieve a\r\n");
						if(motor_run())
						CAN_send_feedback(c_motor_action);	/* 通知主机已经开始一次ACTION */
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
			}
			if(isMotorStatus() == m_stop)
			{
				CAN_send_feedback(c_motor_arrive);	/* 通知主机电机已经到达指定位置 */
			}
		}
}

