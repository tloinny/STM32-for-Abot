#include "motor_gpio.h"

/**
 *@function 步进电机IO初始化
 *CLK	---- PA6
 *DIR	---- PB3
 *EN	---- PB4
 *@param void
 *@return void
 */
void motor_io_init(void)
{
	GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA|RCC_APB2Periph_GPIOB|RCC_APB2Periph_AFIO, ENABLE);  /* 使能GPIO外设 */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_5; /* HOME */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;  /* 上拉输入 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);	/* 初始化GPIO */		
	
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_6; /* TIM_CH1 CLK */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;  /* 复用推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOA, &GPIO_InitStructure);	/* 初始化GPIO */
	GPIO_ResetBits(GPIOA,GPIO_Pin_6);
	
	GPIO_PinRemapConfig(GPIO_Remap_SWJ_JTAGDisable, ENABLE);	/* 关闭JTAG，这样才能将PB3和PB4当做普通IO口使用 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_3; /* DIR */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;  /* 推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);	/* 初始化GPIO */
	GPIO_ResetBits(GPIOB,GPIO_Pin_3);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_4; /* EN */
	GPIO_Init(GPIOB, &GPIO_InitStructure);	/* 初始化GPIO */
	GPIO_SetBits(GPIOB,GPIO_Pin_4);
}
