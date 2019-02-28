/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: DMA底层配置
 */

#include "dma.h"

DMA_InitTypeDef DMA_InitStructure;
u16 DMA1_MEM_LEN;	 /* 保存DMA每次数据传送的长度 */

/**
 *@function DMA1的各通道配置
 *这里的传输形式是固定的,这点要根据不同的情况来修改
 *从存储器->外设模式/8位数据宽度/存储器增量模式
 *@param 
 *			DMA_CHx:DMA通道CHx
 *			cpar:外设地址
 *			cmar:存储器地址
 *			cndtr:数据传输量
 *@return void
 */
void DMA_Config(DMA_Channel_TypeDef* DMA_CHx,u32 cpar,u32 cmar,u16 cndtr)
{
	
 	RCC_AHBPeriphClockCmd(RCC_AHBPeriph_DMA1, ENABLE);	/* 使能DMA钟源 */
	delay_ms(5);
	
  DMA_DeInit(DMA_CHx);   /* 将DMA的通道1寄存器重设为缺省值 */

	DMA1_MEM_LEN=cndtr;
	DMA_InitStructure.DMA_PeripheralBaseAddr = cpar;  /* DMA外设基地址 */
	DMA_InitStructure.DMA_MemoryBaseAddr = cmar;  /* DMA内存基地址 */
	DMA_InitStructure.DMA_DIR = DMA_DIR_PeripheralDST;  /* 数据传输方向，从内存读取发送到外设 */
	DMA_InitStructure.DMA_BufferSize = cndtr;  /* DMA通道的DMA缓存的大小 */
	DMA_InitStructure.DMA_PeripheralInc = DMA_PeripheralInc_Disable;  /* 外设地址寄存器不变 */
	DMA_InitStructure.DMA_MemoryInc = DMA_MemoryInc_Enable;  /* 内存地址寄存器递增 */
	DMA_InitStructure.DMA_PeripheralDataSize = DMA_PeripheralDataSize_HalfWord;  /* 数据宽度为16位 */
	DMA_InitStructure.DMA_MemoryDataSize = DMA_MemoryDataSize_HalfWord; /* 数据宽度为16位 */
	DMA_InitStructure.DMA_Mode = DMA_Mode_Normal;  /* 工作在正常模式 */
	DMA_InitStructure.DMA_Priority = DMA_Priority_High; /* DMA通道 x拥有中优先级 */
	DMA_InitStructure.DMA_M2M = DMA_M2M_Disable;  /* DMA通道x没有设置为内存到内存传输 */
	DMA_Init(DMA_CHx, &DMA_InitStructure);  	
} 

/**
 *@function 进度反馈，返回剩下的数据量
 *@param 
 *			DMA_CHx:DMA通道CHx
 *@return void
 */
u16 DMA_send_feedback(DMA_Channel_TypeDef* DMA_CHx)
{
	return DMA_CHx->CNDTR;
}  

