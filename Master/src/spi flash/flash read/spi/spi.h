#ifndef _SPI_H_
#define _SPI_H_
#include "spi.h"
#include "sys.h"

void SPI2_Init(void);			 /* 初始化SPI */
void SPI2_SetSpeed(u8 SpeedSet); 	/* 设置SPI速度 */
u8 SPI2_ReadWriteByte(u8 TxData);	/* SPI读写一个字节 */

#endif
