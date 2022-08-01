#ifndef IMAGE_H
#define IAMGE_H

#include "common.h"
#include "general.h"
#include "parameter.h"
#include <math.h>
#include <memory.h>

// search
#define SEARCH_LEFT     1
#define SEARCH_RIGHT    (IMAGE_WIDTH - 2)
#define SEARCH_BOT      (MT9V03X_H * 3 / 4)
#define SEARCH_TOP      (MT9V03X_H * 1 / 24)
#define SEARCH_MID      ((SEARCH_BOT - SEARCH_TOP) / 2)
#define SEARCH_STEP_Y	1
#define SEARCH_STEP_X	1
#define SEARCH_MARGIN	7
#define SEARCH_TRY      3

// downsample
#define DOWNSAMPLE_X	5
#define DOWNSAMPLE_Y	5
#define DOWNSAMPLE_H	24
#define DOWNSAMPLE_W	38
#define DOWNSAMPLE_N	(DOWNSAMPLE_W * DOWNSAMPLE_H)
#define DOWNSAMPLE_C	1                           // color
#define DOWNSAMPLE_S	256                         // color


void image_update_roadwidth();
void image_update_thresvalue();
void image_update_err_weight();
void image_calcu_err();
void image_debug();
void image_find_midline(uint8 mode);
uint16 image_fast_otsu();
uint16 image_find_whiteline();
uint16 image_fastsearch_leftline();
uint16 image_fastsearch_rightline();
uint16 image_get_correlation(uint16 flag);


void image_fast_thres();

#endif 
