/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: 外部中断驱动
 */

#include "sys_conf.h"

u8 home_flag = 0;	/* 限位开关标志位 */

/**
 *@function 外部中断初始化
 *@param void
 *@return void
 */
void EXTIX_Init(void)
{
 
 	EXTI_InitTypeDef EXTI_InitStructure;
 	NVIC_InitTypeDef NVIC_InitStructure;

  	RCC_APB2PeriphClockCmd(RCC_APB2Periph_AFIO,ENABLE);	/* 使能复用功能时钟 */

    /* GPIOA.5 中断线以及中断初始化配置   下降沿触发 */
  	GPIO_EXTILineConfig(GPIO_PortSourceGPIOA,GPIO_PinSource5);

  	EXTI_InitStructure.EXTI_Line=EXTI_Line5;
  	EXTI_InitStructure.EXTI_Mode = EXTI_Mode_Interrupt;	
  	EXTI_InitStructure.EXTI_Trigger = EXTI_Trigger_Falling;
  	EXTI_InitStructure.EXTI_LineCmd = ENABLE;
  	EXTI_Init(&EXTI_InitStructure);	 	//根据EXTI_InitStruct中指定的参数初始化外设EXTI寄存器

  	NVIC_InitStructure.NVIC_IRQChannel = EXTI9_5_IRQn;			//使能按键WK_UP所在的外部中断通道
  	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 0x02;	//抢占优先级2， 
  	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0x03;					//子优先级3
  	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;								//使能外部中断通道
  	NVIC_Init(&NVIC_InitStructure); 
}


void EXTI9_5_IRQHandler(void)
{
	delay_ms(10);	/* 消抖 */
	if(GPIO_ReadInputDataBit(GPIOA, GPIO_Pin_5) == 0)	home_flag = 1;	/* 限位开关标志为触发 */
	EXTI_ClearITPendingBit(EXTI_Line5);  /* 清除LINE5上的中断标志位 */  
}
 
