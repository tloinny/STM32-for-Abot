#include "sys_conf.h"

extern u8 home_flag;

const float step_angle = 2 * pi/(motor_type*Micro_Step);
const float timer_frep = 72000000/(psc_init+1);
u8 Motor_status = 0;
int current_position;
int motion_dir = 1;
u32 pre_cndtr = 0;
u32 this_cndtr = 0;
u16 send_buf[send_buf_size];

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
 *@function 电机运动控制
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
u8 motor_move_ready(float steps, u8 dir, float speed_max, float speed_init, float acc_accel, float acc_decel, u16 * S_buf)
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
			if(i == 0) *(S_buf + i) = (u16)(0.5 + timer_frep* sqrt(step_angle/acc_accel) * 0.676);
			if(i > 0 && i < accel_steps)	/* 加速段 */
			{
				*temp = (u16)(0.5+(*(S_buf + i - 1) - ((*(S_buf + i - 1) * 2 + compensation) / (4 * i - 1))));
				if(*temp != 0)
				*(S_buf + i) = (*temp);
				else *(S_buf + i) = 2;
				compensation = (float)fmod( *(S_buf + i - 1) * 2 + compensation, 4 * i - 1);	/* 更新补偿 */
			}
			if(i >= accel_steps && i < accel_steps + const_steps)	/* 匀速段 */
			{
				*(S_buf + i) = arr_max;
			}
			if(i >= accel_steps + const_steps && i < steps)	/* 减速段 */
			{
				*temp = (u16)(0.5+(*(S_buf + (int)steps - i - 1) - (( *(S_buf + (int)steps - i - 1) * 2 + compensation) / (4 * i - 1))));
				if(*temp != 0)
				*(S_buf + i) = (*temp);
				else *(S_buf + i) = 2;
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
		//DMA_Enable(DMA1_Channel6,(u16)steps + 1);
		return 1;
}

/**
 *@function 电机运行
 *@param void
 *@return 
 * 				1:成功发送
 *				0:尚未发送
 */
u8 motor_run()
{
	if(Motor_status != m_moving && send_buf[0] != 0)	/* 只有电机不处于运动状态而且运动步数大于时才开始下一次发送 */
	{
		TIM3->ARR = 2;	/* 由于最后一项是0，所以在最后的时刻ARR会被清零，导致下一次启动无效。*/
		DMA_Cmd(DMA1_Channel6, ENABLE);
		TIM_Cmd(TIM3, ENABLE);  /* 使能TIM3 */
		TIM3->EGR = 0x00000001;
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
	TIM_Cmd(TIM3,DISABLE);
	DMA_Cmd(DMA1_Channel6,DISABLE);
	DMA1_Channel6->CNDTR = 0;
	Motor_status = m_stop;
}

/**
 *@function 电机重启
 *@param void
 *@return void
 */
void motor_restart()
{
	DMA_Enable(DMA1_Channel6, send_buf_size);
	home_flag = 0;
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
		motor_move_ready(1, 0, pi, pi, 1, 1, send_buf);
		motor_run();
		while(1)	/* 如果电机不是处于运动状态，则可以继续发送脉冲 */
		{
			if(MotorStatus() != m_moving) break;
		}
		motor_stop();
	}
	/* 限位开关被触发，电机停止 */
	current_position = 0;
	motor_stop();
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
//	printf("pos:%d\r\n",current_position);
	pre_cndtr = this_cndtr;
	if(this_cndtr == 0 && Motor_status == m_moving)	/* 如果DMA已经发送完数据，而且电机仍然处于运行状态 */
	{
		Motor_status = m_stop;	/* 电机状态切换至停止 */
		CAN_send_feedback(c_motor_arrive);	/* 通知主机电机已经到达指定位置 */
		return m_stop;	/* 认为电机刚刚到达指定位置 */
	}
	if(this_cndtr == 0 && (Motor_status == m_stop || Motor_status == m_waiting))
	{
		Motor_status = m_waiting;
		return m_waiting;	/* 认为电机早已到达指定位置 */
	}
	return m_moving;	/* 否则认为电机尚未到达指定位置 */
}
