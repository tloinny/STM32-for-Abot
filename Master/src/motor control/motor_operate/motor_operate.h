#ifndef _MOTOR_OPERATE_H_
#define _MOTOR_OPERATE_H_
#include "sys.h"
#include "motor_gpio.h"
#include "timer.h"
#include "dma.h"

void motor_init(DMA_Channel_TypeDef*DMA_CHx,u32 cpar,u32 cmar,u16 cndtr, u16 arr, u16 psc);
void motor_enable(void);
void motor_disable(void);
void motor_move(float steps, float speed_max, float speed_init, float acc_accel, float acc_decel, u16 * S_buf);
void motor_home(void);
#endif