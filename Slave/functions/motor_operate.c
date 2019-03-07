/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description:	电机驱动
 */

#include "sys_conf.h"

const float step_angle = 2 * pi/(motor_type*Micro_Step);	/* 步距角 */
const float timer_frep = 72000000/(psc_init+1);	/* 定时器频率 */
u8 Motor_status = 0;	/* 电机状态标志 */
u8 zeroed = 0;	/* 电机归零标志 */
int current_position = -1;	/* 电机虚拟里程计 */
int motion_dir = 1;	/* 电机运动方向标志 */

u32 pre_cndtr = 0;	/* 上一次的CNDTR值 */
u32 this_cndtr = 0;	/* 当前的CNDTR值 */

u16 send_buf[send_buf_size];	/* ARR配置缓存区 */

/**
 *@function 电机初始化
 *电机硬件初始化,和为实现电机控制即将使用的资源进行初始化
 *@param	
 *				DMA_CHx:电机控制占用的DMA通道
 *				cpar:电机控制使用的定时器ARR外设地址
 *				cmar:存储器地址
 *				cndtr:数据传输量
 *				arr:定时器的初始最大重装载值
 *				psc:定时器的分频数
 *@return
 */
void motor_init(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr, u16 arr, u16 psc)
{
	motor_io_init();	/* 初始化IO */
	DMA_Config(DMA_CHx, cpar, cmar, cndtr);	/* 初始化DMA */
	TIM3_PWM_Init(arr, psc);	/* 初始化定时器 */
	motor_enable();					/* 使能电机 */
	Motor_status = m_waiting;	/* 初始化电机状态为等待状态 */
}

/**
 *@function 电机使能
 *@param void
 *@return void
 */
void motor_enable()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);	
	Motor_status = m_waiting;
}

/**
 *@function 电机失能
 *@param void
 *@return void
 */
void motor_disable()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_4);
	Motor_status = m_waiting;
}

/**
 *@function 电机转向
 *@param 
*				dir: 0或1,映射为电机运动方向
 *@return void
 */
void motor_dir(u8 dir)
{
	dir ? (GPIO_SetBits(GPIOB, GPIO_Pin_3),motion_dir = 1) : (GPIO_ResetBits(GPIOB, GPIO_Pin_3),motion_dir = -1);
}

/**
 *@function 电机运动控制，核心：AVR446，电机梯形加减速算法
 *@param 
 *				steps:步进电机运动的步数
 *				dir:步进电机运动的方向
 *				speed_max:目标速度
 *				speed_init:初始速度
 *				acc_accel:加速度
 *				acc_decel:减速度
 *				S_buf:计算结果缓存区
 *@return
 *				1:成功配置send_buf
 *				0:配置过程出现问题
 */
u8 motor_point_movement_ready(float steps, u8 dir, float speed_max, float speed_init, float acc_accel, float acc_decel, u16 * S_buf)
{
	if(((u16)(steps+1.5))>send_buf_size) return 0;	/* 检查边界 */
	u16 max_steps_lim = (u16)(0.5+((speed_max+speed_init)*(speed_max-speed_init))/ (2*step_angle*acc_accel));	/* the number of steps needed to accelerate to the desired speed */
	u16 acc_lim = (u16)(0.5+(steps * (acc_decel/(acc_accel+acc_decel))));		/* the number of steps before deceleration starts */
	u16 arr_max = (u16)((0.5+(step_angle * timer_frep /speed_max)));
	u16 accel_steps = 0;
	u16 *decel_steps = (u16*)malloc(sizeof(u16));
	u16 *temp = (u16*)malloc(sizeof(u16));
	u16 const_steps = 0;
	u16 i = 0;
	float compensation = 0;
		if(max_steps_lim <= acc_lim)	/* 如果可以加速到最大速度 */
		{
			accel_steps = max_steps_lim;
			*decel_steps = accel_steps * (acc_accel/acc_decel);
			const_steps = steps - (accel_steps + *decel_steps); 
			*temp = accel_steps + const_steps;
			free(decel_steps);
		}else	/* 如果无法加速到最大速度 */
		{
			accel_steps = acc_lim;
			free(decel_steps);
		}
		steps = (u16)(0.5+steps);
		while(!(i > steps))
		{
			if(i == 0) *(S_buf + i) = (u16)ceil(timer_frep* sqrt(2*step_angle/acc_accel) * 0.676);
			if(i > 0 && i < accel_steps)	/* 加速段 */
			{
				*temp = (u16)(0.5+(*(S_buf + i - 1) - ((*(S_buf + i - 1) * 2 + compensation) / (4 * i - 1))));
				if(*temp > arr_max)
				*(S_buf + i) = (*temp);
				else *(S_buf + i) = arr_max;
				compensation = (float)fmod( *(S_buf + i - 1) * 2 + compensation, 4 * i - 1);	/* 更新补偿 */
			}
			if(i >= accel_steps && i < accel_steps + const_steps)	/* 匀速段 */
			{
				*(S_buf + i) = arr_max;
			}
			if(i >= accel_steps + const_steps && i < steps)	/* 减速段 */
			{
				*temp = (u16)(0.5+(*(S_buf + (int)steps - i - 1) - (( *(S_buf + (int)steps - i - 1) * 2 + compensation) / (4 * i - 1))));
				if(*temp > arr_max)
				*(S_buf + i) = (*temp);
				else *(S_buf + i) = arr_max;
				compensation = (float)fmod( *(S_buf + (int)steps - i - 1) * 2 + compensation, 4 * i - 1);	/* 更新补偿 */	
			}
			if (i == steps)
			{
				*(S_buf + i) = 0 ;	/* 结束 */
				free(temp);
			}
			++i;
		}
		motor_dir(dir);	/* 配置电机运动方向 */
		DMA_Cmd(DMA1_Channel6, DISABLE);	/* 修改DMA配置之前需确保DMA已经失能，否则无法修改配置 */
		steps>0 ? DMA_SetCurrDataCounter(DMA1_Channel6,(u16)steps + 1) : DMA_SetCurrDataCounter(DMA1_Channel6,0);	/* 提前配置DMA的发送位数，但是暂时不使能DMA */
		pre_cndtr = (u32)steps;
		return 1;
}

void motor_trajectory_config()
{
	
}

/**
 *@function 电机开始运行
 *@param void
 *@return 
 * 				1:成功发送
 *				0:尚未发送
 */
u8 motor_run()
{
	if(Motor_status != m_moving && send_buf[0] != 0 && DMA1_Channel6->CNDTR != 0)	/* 只有电机不处于运动状态而且运动步数大于0时才开始下一次发送 */
	{
		TIM3->ARR = 2;	/* 由于arr在send_buf的最后一项是0，所以在最后的时刻ARR会被清零，导致下一次启动无效，此处应先将arr修改为非零值*/
		DMA_Cmd(DMA1_Channel6, ENABLE);
		TIM_Cmd(TIM3, ENABLE);  /* 使能TIM3 */
		TIM3->EGR = 0x00000001;	/* 允许TIM3操作IO口 */
		Motor_status = m_moving;	/* 切换至运行状态 */
		return 1;
	}
	return 0;
}

/**
 *@function 电机停止
 *@param void
 *@return void
 */
void motor_stop()
{
	TIM_Cmd(TIM3,DISABLE);	/* 先关闭定时器 */
	DMA_Cmd(DMA1_Channel6,DISABLE);	/* 再关闭DMA */
	DMA1_Channel6->CNDTR = 0;	/* 清零CNDTR */
	Motor_status = m_stop;	/* 切换至刚刚停止状态 */
}

/**
 *@function 电机复位
 *@param void
 *@return void
 */
void motor_home()
{
	while(home_flag == 0)	/* 当限位开关没有被触发 */
	{
		/* 逐步向关节原点靠近 */
		motor_point_movement_ready(1, 0, 0.5*pi, 0.5*pi, 0.1, 0.1, send_buf);
		motor_run();
		while(1)	/* 如果电机不是处于运动状态，则可以继续发送脉冲 */
		{
			if(MotorStatus() != m_moving) break;
		}
		motor_stop();
	}
	/* 限位开关被触发，电机停止 */
	motor_stop();	/* 停止电机 */
	current_position = 0;	/* 初始化虚拟里程计，设置虚拟里程计原点，该值由实际机器人的限位开关放置位置决定 */
	zeroed = 1;	/* 标志为已经归零 */
	motion_buf_init();	/* 初始化运动信息缓存区的虚拟原点 */
}

/**
 *@function 判断电机的物理状态
 *@param void
 *@return 
 * 				m_moving 	0x01
 * 				m_stop		0x02
 * 				m_waiting 0x03
 */
u8 MotorStatus()
{
	this_cndtr = DMA_send_feedback(DMA1_Channel6);
	current_position = current_position + motion_dir*(pre_cndtr-this_cndtr);	/* 更新当前位置 */
	pre_cndtr = this_cndtr;
	if(this_cndtr == 0 && Motor_status == m_moving)	/* 如果DMA已经发送完数据，而且电机仍然处于运行状态 */
	{
		Motor_status = m_stop;	/* 电机状态切换至停止 */
		if(zeroed) CAN_send_feedback(c_motor_arrive);	/* 如果电机已经完成零点标定，则开启通知功能 */
		return m_stop;	/* 认为电机刚刚到达指定位置 */
	}
	if(this_cndtr == 0 && (Motor_status == m_stop || Motor_status == m_waiting))
	{
		Motor_status = m_waiting;
		return m_waiting;	/* 认为电机早已到达指定位置，处于等待运行状态 */
	}
	return m_moving;	/* 否则认为电机尚未到达指定位置 */
}

/**
 *@function 初始化运动信息缓存区结构体数组的第一位，即初始化关节运动信息的虚拟原点
 *@param void
 *@return void 
 */
void motion_buf_init()
{
	motion_buf[0].rad = current_position;
	motion_buf[0].dir = 0;
	motion_buf[0].speed_max = 0;
}
