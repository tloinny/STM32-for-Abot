#ifndef __DMA_H
#define	__DMA_H	   
#include "sys_conf.h"

void NVIC_Configuration(void);				    					    
void DMA_Config(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr);
void DMA_Enable(DMA_Channel_TypeDef*DMA_CHx,u16 MEM_LEN);
u16 DMA_send_feedback(DMA_Channel_TypeDef* DMA_CHx);
void DMA1_Channel6_IRQHandler(void);
#endif





