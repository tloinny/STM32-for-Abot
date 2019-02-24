#include "sys_conf.h"

u16 receive_buf[usart_buf_size];	/* 数据接收缓存区 */
extern u8 can_buf[];
u8 rec_buf[8] = {0};

extern u8 DEBUG_Tx_Buff[DEBUG_TX_BUFF_SIZE];
extern u8 DEBUG_Rx_Buff[DEBUG_RX_BUFF_SIZE];
extern __IO u8 DEBUG_Receive_length;

int main(void)
{
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	DEBUG_USARTx_DMA_Config();
//uart_init(115200);
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_Normal);
	LED0 = 1;
	delay_ms(100);
	CAN_Call();
	while(1)
		{
			if(DEBUG_Receive_length > 0)//接收完一帧数据,
			{
				if(DEBUG_Rx_Buff[0]==255 && DEBUG_Rx_Buff[DEBUG_Receive_length-1]==255) //&& DEBUG_Receive_length == DEBUG_Rx_Buff[1]*3)
				//DEBUG_USART_DMA_Tx_Start(DEBUG_Rx_Buff, DEBUG_Receive_length);
				CAN_distribute(DEBUG_Rx_Buff, DEBUG_Receive_length);
				DEBUG_Receive_length = 0;
				DEBUG_RX_Start;//开启下一次接收
			}
		}
}

