#include "sys_conf.h"

const float step_angle = 2 * pi/(motor_type*Micro_Step);
const float timer_frep = 72000000/(psc_init+1);

u16 send_buf[send_buf_size];

void motor_init(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr, u16 arr, u16 psc)
{
	motor_io_init();	/* 初始化IO */
	DMA_Config(DMA_CHx, cpar, cmar, cndtr);	/* 初始化DMA */
	TIM3_PWM_Init(arr, psc);	/* 初始化定时器 */
}

void motor_enable()
{
	GPIO_ResetBits(GPIOB, GPIO_Pin_4);	
}

void motor_disable()
{
	GPIO_SetBits(GPIOB, GPIO_Pin_4);
}

void motor_move(float steps, float speed_max, float speed_init, float acc_accel, float acc_decel, u16 * S_buf)
{
	u16 max_steps_lim = (u16)ceil(((speed_max+speed_init)*(speed_max-speed_init))/ (2*step_angle*acc_accel));	/* the number of steps needed to accelerate to the desired speed */
	u16 acc_lim = (u16)ceil(steps * (acc_decel/(acc_accel+acc_decel)));		/* the number of steps before deceleration starts */
	u16 arr_max = (u16)(step_angle* timer_frep /speed_max);
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
		while(!(i > steps))
		{
			if(i == 0) *(S_buf + i) = (u16)(timer_frep* sqrt(step_angle/acc_accel) * 0.676);
			if(i > 0 && i < accel_steps)	/* 加速段 */
			{
				*temp = (u16)ceil(*(S_buf + i - 1) - ((*(S_buf + i - 1) * 2 + compensation) / (4 * i - 1)));
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
				*temp = (u16)ceil (*(S_buf + (int)steps - i - 1) - (( *(S_buf + (int)steps - i - 1) * 2 + compensation) / (4 * i - 1)));
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
		DMA_Enable(DMA1_Channel6,(u16)steps + 1);
}

void motor_home()
{
	
}
