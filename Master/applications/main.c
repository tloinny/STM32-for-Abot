#include "sys_conf.h"

//u16 receive_buf[usart_buf_size];	/* 数据接收缓存区 */
extern u8 can_buf[];
extern u8 can_rec_buf[];
extern u8 slave_num;

extern u8 DEBUG_Tx_Buff[DEBUG_TX_BUFF_SIZE];
extern u8 DEBUG_Rx_Buff[DEBUG_RX_BUFF_SIZE];
extern __IO u8 DEBUG_Receive_length;

int main(void)
{
	delay_init();
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
	LED_Init();
	DEBUG_USARTx_DMA_Config();
	CAN_Mode_Init(CAN_SJW_1tq,CAN_BS2_8tq,CAN_BS1_9tq,4,CAN_Mode_Normal);
	LED0 = 1;
	delay_ms(1000);
	CAN_Call();
	delay_ms(1000);
	home_all();	/* 所有关节寻找原点 */
	while(1)
		{
			if(DEBUG_Receive_length > 0) /* 接收完一帧数据,进行数据分发 */
			{
				if(DEBUG_Rx_Buff[0]==255 && DEBUG_Rx_Buff[DEBUG_Receive_length-1]==255 && DEBUG_Receive_length == 14)
				CAN_distribute(DEBUG_Rx_Buff, DEBUG_Receive_length);
				DEBUG_Receive_length = 0;
				DEBUG_RX_Start;//开启下一次接收
				CAN_send_cmd(C_READY,slave_all);	/* 通知所有节点做好准备工作 */
				DelayForRespond;
				CAN_send_cmd(C_ACTION,slave_all);
			}
			if(Can_Receive_Msg(can_rec_buf) != 0)
			{
				switch(can_rec_buf[0])
				{
					case 'G':
						
						break;
					case 'R':
								
						break;
					case 'H':

						break;
					case 'A':

						break;
					case 'S':

						break;
					case 'D':
							
						break;
					case 'E':
						break;
				}
			}
		}
}

