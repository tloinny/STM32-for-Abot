#include "w25qxx.h"

u16 W25QXX_TYPE=W25Q80;	/* 配置SPI FLASH型号 */
					
/**
 *@function	初始化SPI FLASH IO
 *@param void
 *@return void
 */					
void W25QXX_Init(void)
{	
  GPIO_InitTypeDef GPIO_InitStructure;
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOB, ENABLE);

	GPIO_InitStructure.GPIO_Pin = GPIO_Pin_12;  /* 片选脚 */
 	GPIO_InitStructure.GPIO_Mode = GPIO_Mode_Out_PP;	/* 推挽输出 */
	GPIO_InitStructure.GPIO_Speed = GPIO_Speed_50MHz;
 	GPIO_Init(GPIOB, &GPIO_InitStructure);
 	GPIO_SetBits(GPIOB,GPIO_Pin_12);
 
  W25QXX_CS=1;	/* 片选拉高，取消选中 */				
	SPI2_Init();	/* 初始化SPI */	  
	SPI2_SetSpeed(SPI_BaudRatePrescaler_2);	/* 18M时钟 */
	W25QXX_TYPE=W25QXX_ReadID();	/* 读取FLASH ID */
}  

/**
 *@function	读取状态寄存器
 *@param void
 *@return 寄存器值
 * BIT 7   6  5  4   3   2   1   0
 *     SPR RV TB BP2 BP1 BP0 WEL BUSY
 * SPR:默认0,状态寄存器保护位,配合WP使用
 * TB,BP2,BP1,BP0:FLASH区域写保护设置
 * WEL:写使能锁定
 * BUSY:忙标记位(1:忙;0:空闲)
 * 默认值:0x00
 */
u8 W25QXX_ReadSR(void)   
{  
	u8 byte=0;   
	W25QXX_CS=0;	/* 片选拉低，使能w25q */                            
	SPI2_ReadWriteByte(W25X_ReadStatusReg);	/* 发送读取寄存器命令 */
	byte=SPI2_ReadWriteByte(0Xff);	/* 读取一个字节 */          
	W25QXX_CS=1;	/* 片选拉高，取消选中 */                           
	return byte;   
} 

/**
 *@function 写状态寄存器
 *@parma 要写入的值
 *@return void
 */
void W25QXX_Write_SR(u8 sr)   
{   
	W25QXX_CS=0;                             
	SPI2_ReadWriteByte(W25X_WriteStatusReg);   
	SPI2_ReadWriteByte(sr);               	
	W25QXX_CS=1;                               	      
}   
   
/**
 *@function 写使能
 *置位WEL	
 *@param void
 *@return void
 */
void W25QXX_Write_Enable(void)   
{
	W25QXX_CS=0;                          	
    SPI2_ReadWriteByte(W25X_WriteEnable); 	  
	W25QXX_CS=1;                               	      
} 

/**
 *@function 写禁止
 *清零WEL	
 *@param void
 *@return void
 */
void W25QXX_Write_Disable(void)   
{  
	W25QXX_CS=0;                            
    SPI2_ReadWriteByte(W25X_WriteDisable);     
	W25QXX_CS=1;                                
} 		
	
/**
 *@function 读取芯片ID
 *@param void
 *@return 芯片ID
 *0XEF13,表示芯片型号为W25Q80  
 *0XEF14,表示芯片型号为W25Q16    
 *0XEF15,表示芯片型号为W25Q32  
 *0XEF16,表示芯片型号为W25Q64 
 *0XEF17,表示芯片型号为W25Q128 	
 */
u16 W25QXX_ReadID(void)
{
	u16 Temp = 0;	  
	W25QXX_CS=0;				    
	SPI2_ReadWriteByte(0x90);
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	    
	SPI2_ReadWriteByte(0x00); 	 			   
	Temp|=SPI2_ReadWriteByte(0xFF)<<8;  
	Temp|=SPI2_ReadWriteByte(0xFF);	 
	W25QXX_CS=1;				    
	return Temp;
}   		    

/**
 *@function 读取SPI FLASH
 *从指定地址开始读取指定长度的数据，并将数据储存到数据存储区
 *@param
 *				pBuffer 数据存储区
 *				ReadAddr 读取的起始地址(24bit)
 *				NumByteToRead 要读取的字节数(max:65535)
 *@return void
 */
void W25QXX_Read(u8* pBuffer,u32 ReadAddr,u16 NumByteToRead)   
{ 
 	u16 i;   										    
	W25QXX_CS=0;                            
	SPI2_ReadWriteByte(W25X_ReadData); 	/* 发送读取命令 */        	
	SPI2_ReadWriteByte((u8)((ReadAddr)>>16));	/* 发送24bit地址 */
  SPI2_ReadWriteByte((u8)((ReadAddr)>>8));   
  SPI2_ReadWriteByte((u8)ReadAddr);   
    for(i=0;i<NumByteToRead;i++)
		{ 
        pBuffer[i]=SPI2_ReadWriteByte(0XFF);   /* 循环读取，直至读取完目标字节数 */
    }
	W25QXX_CS=1;  				    	      
}  

/**
 *@function 在一页内写入少于256个字节
 *在指定地址开始写入指定长度的数据，务必确保地址不越界
 *@param
 *				pBuffer 数据存储区
 *				ReadAddr 写入的起始地址(24bit)
 *				NumByteToRead 要写入的字节数(max:256)
 *@return void
 */
void W25QXX_Write_Page(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)
{
 	u16 i;  
  W25QXX_Write_Enable();                  	
	W25QXX_CS=0;                            	
  SPI2_ReadWriteByte(W25X_PageProgram);	/* 发送写页命令 */      	
  SPI2_ReadWriteByte((u8)((WriteAddr)>>16)); 	/* 发送24bit地址 */
  SPI2_ReadWriteByte((u8)((WriteAddr)>>8));   
  SPI2_ReadWriteByte((u8)WriteAddr);   
    for(i=0;i<NumByteToWrite;i++) SPI2_ReadWriteByte(pBuffer[i]);	/* 循环写入，直至写完目标字节数 */
	W25QXX_CS=1;                      
	W25QXX_Wait_Busy();		/* 等待写入结束 */				   		
} 

/**
 *@function 无检验写SPI FLASH
 *在指定地址开始写入指定长度的数据，务必确保地址不越界，有自动换页功能
 *必须确保所写的地址范围内的数据全部为0xff,否则在非0xff的位置将写入失败
 *@param
 *				pBuffer 数据存储区
 *				ReadAddr 写入的起始地址(24bit)
 *				NumByteToRead 要写入的字节数(max:256)
 *@return void
 */
void W25QXX_Write_NoCheck(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 			 		 
	u16 pageremain;	   
	pageremain=256-WriteAddr%256; 	 	    
	if(NumByteToWrite<=pageremain)pageremain=NumByteToWrite;
	while(1)
	{	   
		W25QXX_Write_Page(pBuffer,WriteAddr,pageremain);
		if(NumByteToWrite==pageremain)break;
	 	else //NumByteToWrite>pageremain
		{
			pBuffer+=pageremain;
			WriteAddr+=pageremain;	

			NumByteToWrite-=pageremain;			  
			if(NumByteToWrite>256)pageremain=256; 
			else pageremain=NumByteToWrite; 	 
		}
	};	    
} 
  
/**
 *@function 写SPI FLASH
 *在指定地址开始写入指定长度的数据，该函数带有擦除操作
 *@param
 *				pBuffer 数据存储区
 *				ReadAddr 写入的起始地址(24bit)
 *				NumByteToRead 要写入的字节数(max:256)
 *@return void
 */
u8 W25QXX_BUFFER[4096];		 
void W25QXX_Write(u8* pBuffer,u32 WriteAddr,u16 NumByteToWrite)   
{ 
	u32 secpos;
	u16 secoff;
	u16 secremain;	   
 	u16 i;    
	u8 * W25QXX_BUF;	  
   	W25QXX_BUF=W25QXX_BUFFER;	     
 	secpos=WriteAddr/4096;  
	secoff=WriteAddr%4096;
	secremain=4096-secoff;
 	//printf("ad:%X,nb:%X\r\n",WriteAddr,NumByteToWrite);
 	if(NumByteToWrite<=secremain)secremain=NumByteToWrite;
	while(1) 
	{	
		W25QXX_Read(W25QXX_BUF,secpos*4096,4096);
		for(i=0;i<secremain;i++)
		{
			if(W25QXX_BUF[secoff+i]!=0XFF)break;  
		}
		if(i<secremain)
		{
			W25QXX_Erase_Sector(secpos);
			for(i=0;i<secremain;i++)
			{
				W25QXX_BUF[i+secoff]=pBuffer[i];	  
			}
			W25QXX_Write_NoCheck(W25QXX_BUF,secpos*4096,4096);

		}else W25QXX_Write_NoCheck(pBuffer,WriteAddr,secremain);	   
		if(NumByteToWrite==secremain)break;
		else
		{
			secpos++;
			secoff=0;

		   	pBuffer+=secremain;
				WriteAddr+=secremain;
		   	NumByteToWrite-=secremain;
			if(NumByteToWrite>4096)secremain=4096;
			else secremain=NumByteToWrite;
		}	 
	};	 
}

/**
 *@function 擦除整个芯片
 *@param void
 *@return void
 */
void W25QXX_Erase_Chip(void)   
{                                   
    W25QXX_Write_Enable();
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                        
    SPI2_ReadWriteByte(W25X_ChipErase);     
		W25QXX_CS=1;                          
		W25QXX_Wait_Busy();   				   	
}   

/**
 *@function 擦除一个扇区
 *@param 
 * 				Dst Addr:扇区地址，根据实际容量设置
 *@return void
 */
void W25QXX_Erase_Sector(u32 Dst_Addr)   
{  
 
 	printf("fe:%x\r\n",Dst_Addr);	  
 	Dst_Addr*=4096;
    W25QXX_Write_Enable();                  
    W25QXX_Wait_Busy();   
  	W25QXX_CS=0;                            	
    SPI2_ReadWriteByte(W25X_SectorErase);      	
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>16));  	
    SPI2_ReadWriteByte((u8)((Dst_Addr)>>8));   
    SPI2_ReadWriteByte((u8)Dst_Addr);  
		W25QXX_CS=1;                            	     
    W25QXX_Wait_Busy();   				   		
}  

/**
 *@function 等待空闲
 *@param void
 *@return void
 */
void W25QXX_Wait_Busy(void)   
{   
	while((W25QXX_ReadSR()&0x01)==0x01);  		
}  

/**
 *@function 进入掉电模式
 *@param void
 *@return void
 */
void W25QXX_PowerDown(void)   
{ 
  	W25QXX_CS=0;                           	 	
    SPI2_ReadWriteByte(W25X_PowerDown);      
		W25QXX_CS=1;                            
    rt_hw_us_delay(3);                            
}   

/**
 *@function 唤醒芯片
 *@param void
 *@return void
 */
void W25QXX_WAKEUP(void)   
{  
  	W25QXX_CS=0;                            
    SPI2_ReadWriteByte(W25X_ReleasePowerDown);	
		W25QXX_CS=1;                         
    rt_hw_us_delay(3);                         
}   

