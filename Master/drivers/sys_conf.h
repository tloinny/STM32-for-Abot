/**
 *@title Abot Firmware
 * Copyright: Copyright (c) 2019 Abot [https://github.com/tloinny/STM32-for-Abot]
 *
 *@created on 2019-1-08  
 *@author:tony-lin
 *@version 1.0.0 
 * 
 *@description: Abot master firmware config file
 */

#ifndef _SYS_CONF_H_
#define _SYS_CONF_H_

/**
 *@description include  
 *在这里定义整个代码include的头文件
 */

#include "stm32f10x.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "can.h"
#include "delay.h"
#include "sys.h"
#include "can_protocol.h"
#include "led.h"
#include "dma_usart1_debug.h"

/**
 *@description define 
 *在这里配置用户代码中的define值
 */
 
/*
 *数组容量设置
 */
#define can_buf_size 8
#define usart_buf_size 1000
#define slave_num_max 6

/*
 *计算参数
 */
#define pi 3.14

/*
 *cmd
 */
#define C_CALL						'C'
#define C_READY						'R'
#define C_ACTION 					'A'
#define C_STOP 						'S'
#define C_HOME 						'H'
#define C_MOTOR_DISABLE		'D'
#define C_MOTOR_ENABLE		'E'


/*
 *ID
 */
#define master 			0x20000000	/* mask: 0x20000000 */
#define slave_0 		0x00200000	/* mask: 0x00200000 */
#define slave_1 		0x00400000	/* mask: 0x00400000 */
#define slave_2 		0x00800000	/* mask: 0x00800000 */
#define slave_3 		0x01000000	/* mask: 0x01000000 */
#define slave_4			0x02000000	/* mask: 0x02000000 */	
#define slave_5			0x04000000	/* mask: 0x04000000 */	
#define slave_all 	0x07Ef0000

#define DelayForRespond delay_ms(6);

/*
 *统一声明变量
 */
extern u8 can_send_buf[];
extern u8 can_rec_buf[];
extern u8 slave_num;
extern u8 slave_buf_available;

extern u8 DEBUG_Tx_Buff[DEBUG_TX_BUFF_SIZE];
extern u8 DEBUG_Rx_Buff[DEBUG_RX_BUFF_SIZE];
extern __IO u8 DEBUG_Receive_length;

#endif
