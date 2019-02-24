#include "timer.h"
#include "sys_conf.h"

TIM_TimeBaseInitTypeDef  TIM_TimeBaseStructure;
 
/*
 *TIM3 PWM部分初始化 
 *PWM输出初始化
 *arr：自动重装值
 *psc：时钟预分频数
 */
void TIM3_PWM_Init(u16 arr,u16 psc)
{  
	GPIO_InitTypeDef GPIO_InitStructure;
	TIM_OCInitTypeDef  TIM_OCInitStructure;
	

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_TIM3, ENABLE);	/* 使能定时器3时钟 */
 	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);  /* 使能GPIO外设 */   
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; /* TIM_CH1*/
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  /* 复用推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);/* 初始化GPIO */
	GPIO_ResetBits(GPIOA, GPIO_Pin_6);
 
   /* 初始化TIM3 */
	TIM_TimeBaseStructure.TIM_Period = arr; /* 设置在下一个更新事件装入活动的自动重装载寄存器周期的值 */
	TIM_TimeBaseStructure.TIM_Prescaler =psc; /* 设置用来作为TIMx时钟频率除数的预分频值 */
	TIM_TimeBaseStructure.TIM_ClockDivision = 0; /* 设置时钟分割:TDTS = Tck_tim */
	TIM_TimeBaseStructure.TIM_CounterMode = TIM_CounterMode_Up;  /*TIM向上计数模式 */
	TIM_TimeBaseInit(TIM3, &TIM_TimeBaseStructure); /* 根据TIM_TimeBaseInitStruct中指定的参数初始化TIMx的时间基数单位 */
	 
	TIM_OCInitStructure.TIM_OCMode = TIM_OCMode_PWM1; /* 选择定时器模式:TIM脉冲宽度调制模式1 */
 	TIM_OCInitStructure.TIM_OutputState = TIM_OutputState_Enable; /* 比较输出使能 */
	TIM_OCInitStructure.TIM_Pulse= 1;
	TIM_OCInitStructure.TIM_OCPolarity = TIM_OCPolarity_High; /* 输出极性:TIM输出比较极性高 */
	TIM_OC1Init(TIM3, &TIM_OCInitStructure);  /* 根据T指定的参数初始化外设TIM3 OC1 */
	TIM3->CR1 = 0x00000081;
	TIM_DMACmd(TIM3, TIM_DMA_CC1, ENABLE);
	TIM_Cmd(TIM3, ENABLE);  /* 使能TIM3 */
}
