/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: CAN底层配置
 */
 
#include "sys_conf.h"

/**
 *@function CAN初始化
 *@param	
 *				tsjw:	重新同步跳跃时间单元，范围:CAN_SJW_1tq~ CAN_SJW_4tq
 *				tbs2:	时间段2的时间单元，范围:CAN_BS2_1tq~CAN_BS2_8tq;
 *				tbs1:	时间段1的时间单元，范围:CAN_BS1_1tq ~CAN_BS1_16tq
 *				brp :	波特率分频器，范围:1~1024;  tq=(brp)*tpclk1
 *							波特率=Fpclk1/((tbs1+1+tbs2+1+1)*brp);
 *							mode:CAN_Mode_Normal,普通模式;CAN_Mode_LoopBack,回环模式;
 *							Fpclk1的时钟在初始化时设置为36M,如果设置CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_LoopBack);
 *							则波特率:36M/((8+9+1)*4)=500Kbps
 *@return	0,初始化成功;
 *   		 其他,初始化失败; 
 */
u8 CAN_Mode_Init(u8 tsjw,u8 tbs2,u8 tbs1,u16 brp,u8 mode)
{ 
	GPIO_InitTypeDef 		GPIO_InitStructure; 
	CAN_InitTypeDef        	CAN_InitStructure;
	CAN_FilterInitTypeDef  	CAN_FilterInitStructure;
#if CAN_RX0_INT_ENABLE 
	NVIC_InitTypeDef  		NVIC_InitStructure;
#endif

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE);/* 使能PORTA时钟	*/                   											 

	RCC_APB1PeriphClockCmd(RCC_APB1Periph_CAN1, ENABLE);/* 使能CAN1时钟 */
	
	/* CAN IO口设置 */
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	/* 复用推挽 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);	/* PA12----CAN_TX */

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_11;
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_IPU;	/* 上拉输入 */
	GPIO_Init(GPIOA, &GPIO_InitStructure);			/* PA11----CAN_RX */

	/* CAN单元设置 */
	CAN_InitStructure.CAN_TTCM=DISABLE;
	CAN_InitStructure.CAN_ABOM=DISABLE;
	CAN_InitStructure.CAN_AWUM=DISABLE;
	CAN_InitStructure.CAN_NART=DISABLE;
	CAN_InitStructure.CAN_RFLM=DISABLE;
	CAN_InitStructure.CAN_TXFP=DISABLE;
	CAN_InitStructure.CAN_Mode= mode;
	
	/* 波特率设置 */
	CAN_InitStructure.CAN_SJW=tsjw;
	CAN_InitStructure.CAN_BS1=tbs1;
	CAN_InitStructure.CAN_BS2=tbs2;
	CAN_InitStructure.CAN_Prescaler=brp; 
	CAN_Init(CAN1, &CAN_InitStructure);	/* 初始化CAN1 */

	CAN_FilterInitStructure.CAN_FilterNumber=0;	/* 过滤器0 */
	CAN_FilterInitStructure.CAN_FilterMode=CAN_FilterMode_IdMask; /* 屏蔽位宽模式 */
	CAN_FilterInitStructure.CAN_FilterScale=CAN_FilterScale_32bit; 	/* 32位宽 */ 
	CAN_FilterInitStructure.CAN_FilterIdHigh=0x2000;	/* 32位ID */
	CAN_FilterInitStructure.CAN_FilterIdLow=0x0000;
	CAN_FilterInitStructure.CAN_FilterMaskIdHigh=0x2000;	/* 32位MASK */
	CAN_FilterInitStructure.CAN_FilterMaskIdLow=0x0000;
	CAN_FilterInitStructure.CAN_FilterFIFOAssignment=CAN_Filter_FIFO0;	/* 过滤器关联到FIFO0 */
	CAN_FilterInitStructure.CAN_FilterActivation=ENABLE;	/* 激活过滤器 */

	CAN_FilterInit(&CAN_FilterInitStructure);			/* 过滤器初始化 */
	
#if CAN_RX0_INT_ENABLE 	/* 如果使能RX0中断 */
	CAN_ITConfig(CAN1,CAN_IT_FMP0,ENABLE);	/* FIFO消息挂号中断允许 */

	NVIC_InitStructure.NVIC_IRQChannel = USB_LP_CAN1_RX0_IRQn;
	NVIC_InitStructure.NVIC_IRQChannelPreemptionPriority = 1; 	/* 主优先级为1 */
	NVIC_InitStructure.NVIC_IRQChannelSubPriority = 0;   /* 次优先级为0 */        
	NVIC_InitStructure.NVIC_IRQChannelCmd = ENABLE;
	NVIC_Init(&NVIC_InitStructure);
#endif
	return 0;
}   
 
#if CAN_RX0_INT_ENABLE	/* 如果使能RX0中断 */		    
void USB_LP_CAN1_RX0_IRQHandler(void)
{
  	CanRxMsg RxMessage;
	int i=0;
    CAN_Receive(CAN1, 0, &RxMessage);
	for(i=0;i<8;i++)
	printf("rxbuf[%d]:%d\r\n",i,RxMessage.Data[i]);
}
#endif

/**
 *@function can发送一组数据
 *固定格式：ID为0x12，标准帧，数据帧
 *@param 
 *				len:数据长度(max 8)		     
 *				msg:数据指针，最大8个字节
 *@return 
 *				0----成功
 *				其他----失败
 */
u8 Can_Send_Msg(u8* msg,u8 len,u32 ID)
{	
	u8 mbox;
	u16 i=0;
	CanTxMsg TxMessage;
	TxMessage.StdId=(ID >> 21);			/* 标准标识符 */
	TxMessage.ExtId=((ID >> 3)&0x3ffff);			/* 设置扩展标识符 */
	TxMessage.IDE=CAN_Id_Standard; 	/* 标准帧 */
	TxMessage.RTR=CAN_RTR_Data;		/* 数据帧 */
	TxMessage.DLC=len;				/* 要发送的数据长度 */
	for(i=0;i<len;i++)
	TxMessage.Data[i]=msg[i];			          
	mbox= CAN_Transmit(CAN1, &TxMessage);   
	i=0; 
	while((CAN_TransmitStatus(CAN1, mbox)==CAN_TxStatus_Failed)&&(i<0XFFF))i++;	/* 等待数据发送 */
	if(i>=0XFFF)return 1;
	return 0;	 
}

/**
 *@function	CAN接收数据查询
 *@param
 *				buf:数据缓存区
 *@return
 *				0----无数据
 *				其他----接收到的数据长度
 */
u8 Can_Receive_Msg(u8 *buf)
{		   		   
 	u32 i;
	CanRxMsg RxMessage;
  if( CAN_MessagePending(CAN1,CAN_FIFO0)==0)return 0;		/* 判断是否接收到数据 */
  CAN_Receive(CAN1, CAN_FIFO0, &RxMessage);	/* 读取数据 */
  for(i=0;i<8;i++)
	{
    buf[i]=RxMessage.Data[i];
	}
	return RxMessage.DLC;	
}
