#include "imparam.h"

// image
extern uint8 g_image[MT9V03X_H][MT9V03X_W];
uint8 g_watch[MT9V03X_H][MT9V03X_W] = { 0 };
#ifndef CAR_3WHEEL
float g_image_pitch = 35.0;
#endif
// road width
float g_road_width_bias = 27.0;
float g_road_width_ratio = 1.19;
// image err
float g_image_err = 0.0;
float g_image_err_p = 8.80;
float g_image_err_d = 16.6;
float g_image_err_pk = 0;
float g_image_err_dp = 0;
// thres
int16 g_differ_threshold1 = 15;
int16 g_differ_threshold2 = 5;
uint16 g_thres_min = 80;
uint16 g_thres_max = 160;
uint16 g_thres_value = 200;
uint8 g_thres_table[256] = { 0 };
// use row
uint16 g_useful_row = 70;
// err limit
int16 g_camera_limit = 0;
float g_camera_limit_value = 0.0;
float g_camera_limit_high = 0.0;
float g_camera_limit_low = 0.0;
// midline
uint16 g_midline_mode = MIDLINE_PATCH;
// line
uint16 g_left_count = 0;
uint16 g_right_count = 0;
uint16 g_left_line[IMAGE_HEIGHT];
uint16 g_right_line[IMAGE_HEIGHT];
// imager err weight
float g_image_err_bias = 100.0;
float g_image_err_ratio = -0.12;
uint16 g_min_weight = 5;
// all white line
uint16 g_allwhite_n = 0;
uint16 g_allwhite_y = 0;
// longest white line
uint16 g_whiteline_n = 0;
uint16 g_whiteline_x = 0;
// correlation
float g_left_correlation = 0;
float g_right_correlation = 0;
float g_correlation_thres = 0.98;
// otsu
uint16 g_otsu_enable = ENABLE;
// search
uint16 g_search_top = SEARCH_TOP;
uint16 g_search_top_max = SEARCH_BOT;
uint16 g_search_top_min = SEARCH_TOP;