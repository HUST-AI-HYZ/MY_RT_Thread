#ifndef _GENERAL_H
#define _GENERAL_H

#define CAR_3WHEEL

#include "common.h"

#define ENABLE          1
#define DISABLE         0

#define DIR_LEFT        0
#define DIR_RIGHT       1

#define LINE_LEFT       0
#define LINE_RIGHT      1

#define LIMIT_NONE      0   // ���޷�
#define LIMIT_UP_PASS   1   // ѡ�������ֵ
#define LIMIT_DOWN_PASS 2   // ѡ��С����ֵ
#define LIMIT_IN_PASS   3   // ѡ��������ֵ��
#define LIMIT_OUT_PASS  4   // ѡ��������ֵ��

// ����
#define MIDLINE_MAX		1
#define MIDLINE_MIN		2
#define MIDLINE_PATCH	3
#define MIDLINE_LEFT	4
#define MIDLINE_RIGHT	5

//���
#define MOTOR_OFF       0
#define MOTOR_ON        1

//ģʽ
#define GUIDE_CAMERA   0
#define GUIDE_INDUCT   1
#define GUIDE_NONE     2

//������
#define BEEP_SHORT  1
#define BEEP_LONG   2
#define BEEP_DOUBLE 3
#define BEEP_HURRY  4
#define BEEP_DECAY  5
#define BEEP_ONCE   6
#define BEEP_LOW    7

//���尴��
#define BUTTON_NONE         0x00

#define BUTTON_UP		    0x01
#define BUTTON_DOWN 	    0x02
#define BUTTON_LEFT	        0x03
#define BUTTON_RIGHT	    0x04
#define BUTTON_OK		    0x05
#define BUTTON_UP_LONG      0x11
#define BUTTON_DOWN_LONG    0x12
#define BUTTON_LEFT_LONG    0x13
#define BUTTON_RIGHT_LONG	0x14
#define BUTTON_OK_LONG		0x15

#define BUTTON_UP2		    0x06
#define BUTTON_DOWN2 	    0x07
#define BUTTON_LEFT2	    0x08
#define BUTTON_RIGHT2	    0x09
#define BUTTON_OK2		    0x0a
#define BUTTON_UP2_LONG     0x16
#define BUTTON_DOWN2_LONG   0x17
#define BUTTON_LEFT2_LONG   0x18
#define BUTTON_RIGHT2_LONG	0x19
#define BUTTON_OK2_LONG		0x1a

//��������
#define TYPE_INT16  1
#define TYPE_UINT16 2
#define TYPE_INT32  3
#define TYPE_UINT32 4
#define TYPE_FLOAT  5
// #define TYPE_INT8   6
// #define TYPE_UINT8  7

//�꺯
#define LIMITER(val, low, high)     ((val)<(low)?(low):((val)>(high)?(high):(val)))

#endif