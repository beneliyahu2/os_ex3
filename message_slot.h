//
// Created by Avigail on Nov-21.
//

#ifndef OS_EX3_MESSAGE_SLOT_H
#define OS_EX3_MESSAGE_SLOT_H

#include <linux/ioctl.h>

#define MAJOR_NUM 220 //todo - change back to 240
#define DEVICE_NAME "msg_slot_device"
#define MSG_SLOT_CHANNEL _IOW(MAJOR_NUM, 0, unsigned long) // set the ioctrl number for assigning channel number

#endif //OS_EX3_MESSAGE_SLOT_H
