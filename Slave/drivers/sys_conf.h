#ifndef _SYS_CONF_H_
#define _SYS_CONF_H_

/**
 *@description include  
 *在这里定义整个代码include的头文件
 */

#include "stm32f10x.h"
#include "usart.h"
#include "stdio.h"
#include "stdlib.h"
#include "math.h"
#include "can.h"
#include "delay.h"
#include "sys.h"
#include "timer.h"
#include "motor_gpio.h"
#include "motor_operate.h"
#include "can_protocol.h"
#include "exti.h"
#include "led.h"

/**
 *@description define 
 *在这里配置用户代码中的define值
 */
 
/*电机参数设置
 */
#define motor_type 400	/* 电机类型，例：200 pulse/r */
#define Micro_Step 2		/* 驱动细分数 例：1/2 */


/*
 *定时器分频数设置 
 *关系到电机的加速曲线
 */
#define psc_init 719
#define arr_init 0

/*
 *数组容量设置
 */
#define send_buf_size 6401
#define motion_buf_size 200
#define CAN_buf_size 8

/*
 *计算参数
 */
#define pi 3.14

/*
 *额外设置
 */
#define ratio 12					/* 机械减速比 例：1/12 */

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
 *feedback
 */
#define c_receive_call			"RC"
#define c_motor_home 				"H0"
#define c_motor_ready 			"R0"
#define c_motor_arrive			"AR0"
#define c_motor_action			"AC0"
#define c_motor_stop				"S0"
#define c_motor_disable			"D0"
#define c_motor_enable			"E0"

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

/*
 *Motor status
 */
#define m_moving 	0x01	/* 电机正在运动，处于运动状态 */
#define m_stop		0x02	/* 电机刚刚停止运动，处于停止状态 */
#define m_waiting 0x03	/* 电机早已停止运动，处于等待状态 */

#endif
