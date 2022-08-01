#include "parameter.h"

float g_angular_camera_err = 0.0;

// ramp
uint16 g_ramp_state = 0;
uint16 g_ramp_enable = ENABLE;

// cross
uint16 g_cross_enable = ENABLE;
uint16 g_cross_state = 0;
uint16 g_cross_direction = 0;

// fork
uint16 g_fork_state = 0;
uint16 g_fork_direction = DIR_LEFT;
uint16 g_fork_enable = ENABLE;

// ring
uint16 g_ring_state = 0;
uint16 g_ring_enable = ENABLE;
uint16 g_ring_result = 0;

// beep
uint16 g_beep = ENABLE;
uint16 g_beep_status = 0;
uint16 g_beep_set = 0;
