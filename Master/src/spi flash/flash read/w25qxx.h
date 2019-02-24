#ifndef _W25QXX_H_
#define _W25QXX_H_
#include "spi.h"
#include "stdio.h"
#include <rtthread.h>
#include <rthw.h>

#define W25Q80 	0XEF13 	
#define W25Q16 	0XEF14
#define W25Q32 	0XEF15
#define W25Q64 	0XEF16
#define W25Q128	0XEF17

extern u16 W25QXX_TYPE;					/* 定义芯片型号 */

#define	W25QXX_CS 		PBout(12)  		/* W25QXX片选脚信号 */
		 
/* 指令表 */
#define W25X_WriteEnable		0x06 
#define W25X_WriteDisable		0x04 
#define W25X_ReadStatusReg		0x05 
#define W25X_WriteStatusReg		0x01 
#define W25X_ReadData			0x03 
#define W25X_FastReadData		0x0B 
#define W25X_FastReadDual		0x3B 
#define W25X_PageProgram		0x02 
#define W25X_BlockErase			0xD8 
#define W25X_SectorErase		0x20 
#define W25X_ChipErase			0xC7 
#define W25X_PowerDown			0xB9 
#define W25X_ReleasePowerDown	0xAB 
#define W25X_DeviceID			0xAB 
#define W25X_ManufactDeviceID	0x90 
#define W25X_JedecDeviceID		0x9F 

void W25QXX_Init(void);
u16  W25QXX_ReadID(void);  	    		
u8	 W25QXX_ReadSR(void);        	
void W25QXX_Write_SR(u8 sr);  		
void W25QXX_Write_Enable(void);  		
void W25QXX_Write_Disable(void);	
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead);   
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite);
void W25QXX_Erase_Chip(void);    	  	
void W25QXX_Erase_Sector(u32 Dst_Addr);	
void W25QXX_Wait_Busy(void);           
void W25QXX_PowerDown(void);        
void W25QXX_WAKEUP(void);			

#endif
