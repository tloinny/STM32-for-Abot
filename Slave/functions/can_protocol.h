#ifndef _CAN_PROTOCOL_H_
#define _CAN_PROTOCOL_H_
#include "sys_conf.h"

u8 CAN_send_feedback(u8 *feedback);
void clean_can_send_buf(void);

#endif
