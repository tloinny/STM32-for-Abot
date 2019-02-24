#ifndef _CAN_PROTOCOL_H_
#define _CAN_PROTOCOL_H_
#include "sys_conf.h"

u8 CAN_send_motion_info(float rad, float speed, u32 ID);
u8 CAN_send_cmd(u8 cmd, u32 ID);
u8 CAN_distribute(u8 * buf, u8 len);
void CAN_Call(void);
void clean_can_buf(void);
#endif
