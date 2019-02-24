#include "spi.h"

/**
 *@function	SPI初始化
 *@param void
 *@return void
 */
void SPI2_Init(void)
{
 	GPIO_InitTypeDef GPIO_InitStructure;
  SPI_InitTypeDef  SPI_InitStructure;

	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);	/* 使能PORTB时钟 */
	RCC_APB1PeriphClockCmd(RCC_APB1Periph_SPI2,  ENABLE);	/* 使能SPI2时钟 */		
 
	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;	/* PB13,PB14,PB15 */
	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_AF_PP;	/* 复用推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init(GPIOB, &GPIO_InitStructure);

 	GPIO_SetBits(GPIOB,GPIO_Pin_13|GPIO_Pin_14|GPIO_Pin_15);	/* 上拉PB13,PB14,PB15 */

	SPI_InitStructure.SPI_Direction = SPI_Direction_2Lines_FullDuplex;	/* 设置为双线双向全双工 */
	SPI_InitStructure.SPI_Mode = SPI_Mode_Master;	/* 主机模式 */
	SPI_InitStructure.SPI_DataSize = SPI_DataSize_8b;	/* 8位帧结构 */
	SPI_InitStructure.SPI_CPOL = SPI_CPOL_High;
	SPI_InitStructure.SPI_CPHA = SPI_CPHA_2Edge;
	SPI_InitStructure.SPI_NSS = SPI_NSS_Soft;	/* 硬件片选 */
	SPI_InitStructure.SPI_BaudRatePrescaler = SPI_BaudRatePrescaler_256;	/* 波特率分频值为256 */
	SPI_InitStructure.SPI_FirstBit = SPI_FirstBit_MSB;	
	SPI_InitStructure.SPI_CRCPolynomial = 7;
	SPI_Init(SPI2, &SPI_InitStructure); 
 
	SPI_Cmd(SPI2, ENABLE); /* 使能外设 */
	
	SPI2_ReadWriteByte(0xff);	/* 启动传输 */
}

/**
 *@function	设置SPI速度
 *@param		
 *					SPI_BaudRatePrescaler_2   2分频 
 *					SPI_BaudRatePrescaler_8   8分频  
 *					SPI_BaudRatePrescaler_16  16分频 
 *					SPI_BaudRatePrescaler_256 256分频
 *@return void
 */
void SPI2_SetSpeed(u8 SPI_BaudRatePrescaler)
{
  assert_param(IS_SPI_BAUDRATE_PRESCALER(SPI_BaudRatePrescaler));
	SPI2->CR1&=0XFFC7;
	SPI2->CR1|=SPI_BaudRatePrescaler;
	SPI_Cmd(SPI2,ENABLE); 
} 

/**
 *@function	读写一个字节
 *@param	要写入的字节
 *@return	读取的字节
 */
u8 SPI2_ReadWriteByte(u8 TxData)
{		
	u8 retry=0;				 	
	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_TXE) == RESET)	/* 发送缓存空标志位 */
		{
		retry++;
		if(retry>200)return 0;
		}			  
	SPI_I2S_SendData(SPI2, TxData);
	retry=0;

	while (SPI_I2S_GetFlagStatus(SPI2, SPI_I2S_FLAG_RXNE) == RESET)	/* 接受缓存非空标志位 */
		{
		retry++;
		if(retry>200)return 0;
		}	  						    
	return SPI_I2S_ReceiveData(SPI2);    
}
