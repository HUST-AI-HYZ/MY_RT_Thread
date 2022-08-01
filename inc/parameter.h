#ifndef _PARAMETER_H
#define _PARAMETER_H

#include "common.h"
#include "general.h"
#include "imparam.h"

extern uint8 g_image[MT9V03X_H][MT9V03X_W];
extern uint8 g_watch[MT9V03X_H][MT9V03X_W];

extern float g_angular_camera_err;

extern uint16 g_ramp_state;
extern uint16 g_ramp_enable;

extern uint16 g_cross_enable;
extern uint16 g_cross_state;
extern uint16 g_cross_direction;

extern uint16 g_fork_state;
extern uint16 g_fork_direction;
extern uint16 g_fork_enable;

extern uint16 g_ring_state;
extern uint16 g_ring_direction;
extern uint16 g_ring_enable;

extern uint16 g_beep;
extern uint16 g_beep_status;
extern uint16 g_beep_set;

#endif