#include "image.h"
#include "stdlib.h"

#define MAX(a, b) (a > b ? a : b)
#define MIN(a, b) (a > b ? b : a)
#define ABS(a) (a > 0 ? a : -a)
#define DELT(a, b) (a - b)
#define ABS_DELT(a, b) (a > b ? a - b : b - a)
#define DIFFER(a, b) ((a - b) * 100 / (a + b))
#define FAST_DIFFER(a, b) (((a - b) << 7) / (a + b + 1))
#define ABS_DIFFER(a, b) a >= b ? (((a - b) << 7) / (a + b + 1) \
								   : ((b - a) << 7) / (a + b + 1))

typedef struct Line
{
	uint16 from;
	uint16 to;
	uint16 count;
	struct Line* next;
}Line;

static int16 g_mid_line[IMAGE_HEIGHT];
static uint16 g_err_weight[IMAGE_HEIGHT];
static uint16 x_conditon_left = 0;
static uint16 x_conditon_right = 0;

// 更新二值化查找表
void image_update_thresvalue()
{
	static uint8 last_threshold_value = 0;

	// 如果改变才更新，优化加速
	if (last_threshold_value != g_thres_value)
	{
		for (uint16 i = 0; i < g_thres_value; i++)
		{
			g_thres_table[i] = 0;
		}
		for (uint16 i = g_thres_value; i < 256; i++)
		{
			g_thres_table[i] = 1;
		}
		last_threshold_value = g_thres_value;
	}
}





// 寻找中线
void image_find_midline(uint8 mode)
{
	for (uint16 y = SEARCH_BOT; y >= g_search_top; y -= SEARCH_STEP_Y)
	{
		g_mid_line[y] = 0xfff;
	}
	if (mode == 1)
	{
		for (uint16 y = SEARCH_BOT; y >= g_search_top; y -= SEARCH_STEP_Y)
		{
			if (g_left_line[y] && g_right_line[y])
			{
				g_mid_line[y] = (g_right_line[y] + g_left_line[y]) >> 1;
			}
			else if (g_left_line[y])
			{
				g_mid_line[y] = (g_left_line[y] + SEARCH_RIGHT) >> 1;
			}
			else if (g_right_line[y])
			{
				g_mid_line[y] = (g_right_line[y] + SEARCH_LEFT) >> 1;
			}
			else
			{
				g_mid_line[y] = 0xfff;
			}
		}
	}
	else if (mode == 2)
	{
		for (uint16 y = SEARCH_BOT; y >= g_search_top; y -= SEARCH_STEP_Y)
		{
			if (g_left_line[y] && g_right_line[y])
			{
				g_mid_line[y] = (g_right_line[y] + g_left_line[y]) >> 1;
			}
			else
			{
				g_mid_line[y] = 0xfff;
			}
		}
	}
	else if (mode == 3)
	{
		for (uint16 y = SEARCH_BOT; y >= g_search_top; y -= SEARCH_STEP_Y)
		{
			if (g_left_line[y] && g_right_line[y])
			{
				g_mid_line[y] = (g_right_line[y] + g_left_line[y]) >> 1;
			}
			else if (g_left_line[y])
			{
				int16 mid = g_left_line[y] + (uint16)(g_road_width_bias + y * g_road_width_ratio);
				g_mid_line[y] = MIN(mid, SEARCH_RIGHT);
			}
			else if (g_right_line[y])
			{
				int16 mid = g_right_line[y] - (uint16)(g_road_width_bias + y * g_road_width_ratio);
				g_mid_line[y] = MAX(mid, SEARCH_LEFT);
			}
			else
			{
				g_mid_line[y] = 0xfff;
			}
		}
	}
}

// 调试
void image_debug()
{
	memset(g_watch, 0, IMAGE_HEIGHT * IMAGE_WIDTH);

	for (uint16 y = SEARCH_BOT; y >= g_search_top; y -= 3)
	{
		g_watch[y][g_whiteline_x] = 6;
	}
	for (uint16 y = SEARCH_BOT; y >= g_search_top; y -= SEARCH_STEP_Y)
	{
		if (g_left_line[y])
		{
			g_watch[y][g_left_line[y]] = 3;
			//g_watch[y][g_left_line[y] + 1] = 3;
		}
		if (g_right_line[y])
		{
			g_watch[y][g_right_line[y]] = 4;
			//g_watch[y][g_right_line[y] - 1] = 4;
		}
		if (g_mid_line[y] != 0xfff)
		{
			uint16 mx = g_mid_line[y] < SEARCH_LEFT ? SEARCH_LEFT
				: g_mid_line[y] > SEARCH_RIGHT ? SEARCH_RIGHT
				: g_mid_line[y];
			g_watch[y][mx] = 2;
			//g_watch[y][mx + 1] = 2;
		}
	}
	for (uint16 x = 0; x < IMAGE_WIDTH; x += 3)
	{
		g_watch[SEARCH_TOP][x] = 5;
		g_watch[SEARCH_BOT][x] = 5;
		g_watch[g_search_top][x] = 7;
		g_watch[g_search_top + 30][x] = 7;
	}
}

// 类内方差OTSU最佳阈值
uint16 image_fast_otsu()
{
	if (!g_otsu_enable)
	{
		return 0;
	}

	uint16 histogram[DOWNSAMPLE_S] = { 0 };
	uint32 histstd[DOWNSAMPLE_S] = { 0 };

	// 直方图统计
	for (uint16 y = 0; y < IMAGE_HEIGHT; y += DOWNSAMPLE_Y)
	{
		for (uint16 x = SEARCH_LEFT; x <= SEARCH_RIGHT; x += DOWNSAMPLE_X)
		{
			histogram[g_image[y][x] / DOWNSAMPLE_C]++;
		}
	}

	//uint32 background_std = 0;
	//uint32 foreground_std = 0;
	uint16 background_mean = 0;
	uint16 foreground_mean = 0;
	uint32 background_sum = 0;
	uint32 foreground_sum = 0;
	uint16 background_n = 0;
	uint16 foreground_n = 0;
	uint16 i = g_thres_min / DOWNSAMPLE_C;

	// 初始化积分表
	for (uint16 k = 0; k < i; k++)
	{
		background_n += histogram[k];
		background_sum += histogram[k] * k;
	}

	for (uint16 k = i; k < DOWNSAMPLE_S; k++)
	{
		foreground_n += histogram[k];
		foreground_sum += histogram[k] * k;
	}

	// 遍历计算类间方差
	for (; i <= g_thres_max / DOWNSAMPLE_C; i++)
	{
		background_n += histogram[i];
		foreground_n -= histogram[i];

		if (!background_n)
		{
			continue;
		}
		if (!foreground_n)
		{
			break;
		}
		if (!histogram[i])
		{
			histstd[i] = histstd[i - 1];
		}

		background_sum += histogram[i] * i;
		foreground_sum -= histogram[i] * i;

		background_mean = background_sum / background_n;
		foreground_mean = foreground_sum / foreground_n;

		histstd[i] = background_n * foreground_n * (int32)(background_mean - foreground_mean) * (int32)(background_mean - foreground_mean);
	}

	uint32 temp = 0x00;
	uint16 thres = 0;
	uint16 thres_n = 0;

	// 寻找方差最小的灰度级，如果有多个则取平均
	for (uint16 i = g_thres_min / DOWNSAMPLE_C; i <= g_thres_max / DOWNSAMPLE_C; i++)
	{
		if (histstd[i])
		{
			if (histstd[i] > temp)
			{
				temp = histstd[i];
				thres = i;
				thres_n = 1;
			}
			else if (histstd[i] == temp)
			{
				thres += i;
				thres_n++;
			}
		}
	}

	static uint16 flag = 0;
	static float last_thres = 0.0;

	// 防止无效结果
	if (thres)
	{
		// 首次进入更新记录阈值
		if (!flag)
		{
			last_thres = thres / (float)thres_n;
			flag = 1;
		}

		last_thres = 0.95 * last_thres + 0.05 * thres / (float)thres_n;

		g_thres_value = (uint16)(last_thres * DOWNSAMPLE_C);
	}

	return g_thres_value;
}

uint16 image_find_whiteline()
{
	/***********************************************
	 * 第一部分：搜素图像底部白色线段，作为搜素区间
	***********************************************/

	Line node_head = { 0 };		// 链表头
	Line* node = NULL;			// 链表节点
	uint16 node_num = 0;		// 链表长度
	uint8* src = NULL;			// 图像指针

	// 重置search_top
	g_search_top = SEARCH_TOP;

	// 搜索图像底部白色线段
	node_head.next = NULL;
	node = &node_head;
	node_num = 0;
	src = g_image[SEARCH_BOT];
	uint16 x = SEARCH_LEFT;
	while (x <= SEARCH_RIGHT)
	{
		uint16 cnt = 0;
		uint16 start = 0;
		//uint16 end = 0;

		// 如果搜到第一个白色，线段开始
		for (; x <= SEARCH_RIGHT; x += DOWNSAMPLE_X)
		{
			// 
			if (g_thres_table[*(src + x)])
			{
				cnt++;
				start = (x - SEARCH_LEFT) / DOWNSAMPLE_X;
				x += DOWNSAMPLE_X;
				break;
			}
		}

		// 如果搜到第一个黑色，线段结束
		for (; x <= SEARCH_RIGHT; x += DOWNSAMPLE_X)
		{
			if (!g_thres_table[*(src + x)])
			{
				break;
			}
			else
			{
				cnt++;
			}
		}

		// 过滤长度较小的无效线段
		if (cnt >= 5)
		{
			node->next = (Line*)malloc(sizeof(Line));
			// 如果成功开辟，进行储存
			if (node->next)
			{
				node_num++;
				node = node->next;
				node->from = start;
				node->to = start + (cnt - 1);
				node->count = cnt;
				node->next = NULL;
			}
			// 否则返回错误，并退出释放
			else
			{
				return 0xff;
			}
		}
		else
		{
			continue;
		}
	}

	// 选择最长的白色线段，作为白列搜索区间
	Line* best_node = NULL;
	uint16 best_flag = 0;
	uint16 max_count = 0;
	uint16 x_from = 0;
	uint16 x_to = 0;
	// 如果存在白色线段
	if (node_num)
	{
		node = node_head.next;
		while (node)
		{
			if (node->count > max_count)
			{
				max_count = node->count;
				best_node = node;
				best_flag = 1;
			}
			node = node->next;
		}
		x_from = best_node->from;
		x_to = best_node->to;
	}
	// 如果一条白色线段都不存在，已经无法继续进行，则退出
	else
	{
		return 0;
	}

	// 释放记录
	node = node_head.next;
	while (node)
	{
		Line* temp = node->next;
		free(node);
		node = temp;
	}
	node_head.next = NULL;
	best_node = NULL;

	/***********************************************
	 * 第二部分：统计列长
	***********************************************/

	// 统计白列的长度
	uint16 white_sum[DOWNSAMPLE_W + 1] = { 0 };
	uint16 white_mid_x = 0;    // 最长白列位置
	uint16 white_max = 0;  // 白线的最大长度值
	uint16 white_n = 0;    // 最长白列的数量
	for (uint16 x = x_from; x <= x_to; x += 1)
	{
		uint16* sum = &white_sum[x];
		uint8* src = &g_image[0][x * DOWNSAMPLE_X + SEARCH_LEFT];
		// 从下往上搜白色像素的数量
		for (uint16 y = SEARCH_BOT; y >= SEARCH_TOP; y -= DOWNSAMPLE_Y)
		{
			if (g_thres_table[*(src + IMAGE_WIDTH * y)])
			{
				*sum += 1;
			}
			else
			{
				// 如果搜到黑色则停止,并记录最大值
				break;
			}
		}
		if (*sum > white_max)
		{
			white_max = *sum;
			white_mid_x = x;
			white_n = 1;
		}
		else if (*sum == white_max)
		{
			white_mid_x += x;
			white_n++;
		}
	}

	// 统计黑列的长度
	uint16 black_sum[DOWNSAMPLE_W + 1] = { 0 };
	uint16 black_mid_x = 0;
	uint16 black_max = 0;
	uint16 black_n = 0;
	for (uint16 x = x_from; x <= x_to; x += 1)
	{
		uint16* sum = &black_sum[x];
		uint8* src = &g_image[0][x * DOWNSAMPLE_X + SEARCH_LEFT];
		// 从下往上搜白色像素的数量
		for (uint16 y = SEARCH_TOP; y <= SEARCH_BOT; y += DOWNSAMPLE_Y)
		{
			if (!g_thres_table[*(src + IMAGE_WIDTH * y)])
			{
				*sum += 1;
			}
			else
			{
				// 如果搜到黑色则停止,并记录最大值
				break;
			}
		}
		if (*sum > black_max)
		{
			black_max = *sum;
			black_mid_x = x;
			black_n = 1;
		}
		else if (*sum == black_max)
		{
			black_mid_x += x;
			black_n++;
		}
	}

	/***********************************************
	 * 第三部分：计算全白行
	***********************************************/

	// 计算图像上半区域全白行，用于判断十字
	uint16 all_white_n = 0;
	uint16 all_white_y = 0;
	for (uint16 y = 70; y >= SEARCH_TOP; y -= DOWNSAMPLE_Y) // 设置全白行的搜素范围
	{
		if (g_thres_table[g_image[y][SEARCH_LEFT]] && g_thres_table[g_image[y][SEARCH_RIGHT]])
		{
			all_white_y += y;
			all_white_n++;
		}
	}
	g_allwhite_n = all_white_n;

	// 计算全白行的均值y
	if (all_white_n)
	{
		all_white_y = all_white_y / all_white_n;
	}
	else
	{
		all_white_y = 0;
	}
	g_allwhite_y = all_white_y;

	/***********************************************
	 * 第四部分：坡道检测
	***********************************************/

	// 检测坡道，如果最长白列数大于阈值，最长白列长度大于阈值
	uint16 ramp_flag = 0;
	if (
		g_ramp_enable
		&& (white_n >= 16 && white_max >= 17 && x_to - x_from + 1 >= 35 && g_allwhite_n <= 10))
	{
		if (ABS_DELT((white_mid_x / white_n), (DOWNSAMPLE_W / 2)) <= 5)
		{
			// 判断图像中央范围是否存在非最长白列
			uint16 noramp_flag = 0;
			uint16 x0 = MAX(x_from, 13);
			uint16 x1 = MIN(x_to, 25);
			for (uint16 x = x0; x <= x1; x += 1)
			{
				// 如果图像中央范围存在非最长白列，则视为岔道，停止判断
				if (white_sum[x] != white_max)
				{
					noramp_flag = 1;
					break;
				}
			}
			if (noramp_flag)
			{
				ramp_flag = 0;
			}
			else
			{
				ramp_flag = 1;
			}
		}
	}

	/***********************************************
	 * 第五部分：岔道检测
	***********************************************/

	// 检测岔道
	int16 min_idx = 0;		// 最短白列索引x
	uint16 fork_flag = 0;
	if (
		g_fork_enable
		&& (g_allwhite_n >= 2 && x_to - x_from + 1 >= 33)
		&& white_max >= 3)
	{
		// 首先判断图像中央范围是否存在最长白列
		uint16 nofork_flag = 0;
		uint16 x0 = MAX(x_from, 13);
		uint16 x1 = MIN(x_to, 25);
		for (uint16 x = x0; x <= x1; x += 1)
		{
			// 如果图像中央范围存在最长白列，则视为十字，停止判断
			if (white_sum[x] == white_max)
			{
				nofork_flag = 1;
				break;
			}
		}

		if (!nofork_flag)
		{
			// 搜索范围
			uint16 x_start = 0;
			uint16 x_end = 0;
			uint16 x_mid = (x_from + x_to) / 2;
			uint16 x = 0;
			uint16 fork_thres = 0;
			fork_thres = MAX(((int16)white_max - 8), 4);	// 设置动态阈值
			for (x = x_from; x <= x_mid; x++)
			{
				if (white_sum[x] >= fork_thres)
				{
					break;
				}
			}
			x_start = x;
			for (x = x_to; x >= x_mid; x--)
			{
				if (white_sum[x] >= fork_thres)
				{
					break;
				}
			}
			x_end = x;
			uint16 x_len = x_end - x_start;

			// 寻找最小值
			uint16 min = 0xff;
			uint16 min_n = 0;
			for (x = x_start; x <= x_end; x++)
			{
				if (white_sum[x] < min)
				{
					min = white_sum[x];
					min_idx = x;
					min_n = 1;
				}
				else if (white_sum[x] == min)
				{
					min_idx += x;
					min_n += 1;
				}
			}
			min_idx /= min_n;

			// 最小值允许区间    
			if (min_idx - x_start < (x_len * 1 / 8) || (x_end - min_idx) < (x_len * 1 / 8) || min <= 3)
			{
				nofork_flag = 1;
			}

			if (!nofork_flag)
			{
				// 判断图像是否为三角形岔道口
				uint16 range = (x_end - x_start + 1) / 3;
				int16 y_sum_left = 0;
				int16 y_sum_mid = 0;
				int16 y_sum_right = 0;
				// 累加分区
				for (uint16 i = 0; i <= range; i++)
				{
					y_sum_left += white_sum[x_start + i];
					y_sum_right += white_sum[x_end - i];
				}
				for (uint16 i = 0; i < range / 2; i++)
				{
					if (min_idx + i < x_end)
					{
						y_sum_mid += white_sum[min_idx + i];
					}
					else
					{
						y_sum_mid += white_sum[x_end];
					}
					if (min_idx - i > x_start)
					{
						y_sum_mid += white_sum[min_idx - i];
					}
					else
					{
						y_sum_mid += white_sum[x_end];
					}
				}
				y_sum_left = (y_sum_left << 4) / range;
				y_sum_right = (y_sum_right << 4) / range;
				if (range % 2)
				{
					y_sum_mid = (y_sum_mid << 4) / (range - 1);
				}
				else
				{
					y_sum_mid = (y_sum_mid << 4) / range;
				}

				// 如果符合三角形
				if ((y_sum_left - y_sum_mid) > 0 && (y_sum_right - y_sum_mid) > 0)
				{
					if (g_fork_direction == DIR_LEFT)
					{
						x_from = x_start;
						x_to = min_idx;
					}
					else
					{
						x_from = min_idx;
						x_to = x_end;
					}
					fork_flag = 1;
				}
			}
		}
	}

	// 如果是岔道，则需重新寻找最长白列
	if (fork_flag)
	{
		uint16 max = 0;
		white_mid_x = 0;
		white_n = 0;
		for (uint16 x = x_from; x <= x_to; x += 1)
		{
			if (white_sum[x] > max)
			{
				white_mid_x = x;
				max = white_sum[x];
				white_n = 1;
			}
			else if (white_sum[x] == max)
			{
				white_mid_x += x;
				white_n++;
			}
		}
	}

	/***********************************************
	 * 第六部分：计算最长白列x
	***********************************************/

	white_mid_x = white_mid_x / white_n;
	g_whiteline_n = white_n;
	g_whiteline_x = white_mid_x * DOWNSAMPLE_X + SEARCH_LEFT;

	/***********************************************
	 * 第七部分：计算搜线加速条件
	***********************************************/

	// 计算搜线加速条件
	if (fork_flag)
	{
		if (g_fork_direction == DIR_LEFT)
		{
			x_conditon_left = (x_from + x_to) / 2 * DOWNSAMPLE_X + SEARCH_LEFT;
			x_conditon_right = x_to * DOWNSAMPLE_X + SEARCH_LEFT;
		}
		else
		{
			x_conditon_left = x_from * DOWNSAMPLE_X + SEARCH_LEFT;
			x_conditon_right = (x_from + x_to) / 2 * DOWNSAMPLE_X + SEARCH_LEFT;
		}
	}
	else
	{
		x_conditon_left = SEARCH_LEFT + 1 * (g_whiteline_x - SEARCH_LEFT) / 2;
		x_conditon_right = SEARCH_RIGHT - 1 * (SEARCH_RIGHT - g_whiteline_x) / 2;
	}

	/***********************************************
	 * 第八部分：计算search_top
	***********************************************/

	// 计算有效搜索范围
	uint16 search_top = SEARCH_TOP;
	src = &g_image[0][g_whiteline_x];
	for (uint16 y = SEARCH_BOT - white_sum[white_mid_x] * DOWNSAMPLE_Y / 2; y >= SEARCH_TOP; y -= SEARCH_STEP_Y)
	{
		// 如果出现第一个黑点
		if (!g_thres_table[*(src + IMAGE_WIDTH * y)])
		{
			// 选择上一行
			search_top = y + 1;
			break;
		}
	}
	g_search_top = LIMITER(search_top, g_search_top_min, g_search_top_max);

	/***********************************************
	 * 第九部分：检测十字
	***********************************************/

	// 如果是岔道，再次检测是否被误判未十字路口
	if (fork_flag)
	{
		if (ABS_DELT(g_whiteline_x, (IMAGE_WIDTH / 2)) <= 20)
		{
			fork_flag = 0;
		}
		else
		{
			;
		}
	}

	uint16 cross_flag = 0;
	if (
		g_cross_enable
		&& g_search_top <= 50
		&& (g_whiteline_n < 22 && ABS_DELT(g_whiteline_x, (IMAGE_WIDTH / 2)) <= 30)
		&& (g_allwhite_n >= 5 && g_allwhite_y >= 50))
	{
		cross_flag = 1;
	}

	/***********************************************
	 * finish
	***********************************************/

	if (ramp_flag)
	{
		g_beep_set = BEEP_HURRY;
		return 1;
	}
	else if (fork_flag)
	{
		g_beep_set = BEEP_DECAY;
		return 2;
	}
	else if (cross_flag)
	{
		g_beep_set = BEEP_LOW;
		return 3;
	}
	else
	{
		return 0;
	}
}

uint16 image_fastsearch_leftline()
{
	int16 end_x = SEARCH_LEFT;
	uint16 count = 0;
	uint16 times = 0;
	uint16 try1 = SEARCH_TRY;
	uint16 diff_thres = 0;
	uint16 find_flag = 0x0000;
	uint16 last_x = g_whiteline_x;
	uint16 margin = SEARCH_MARGIN;
	uint8* thres_table = g_thres_table;

	for (uint16 y = SEARCH_BOT; y >= SEARCH_TOP; y -= SEARCH_STEP_Y)
	{
		g_left_line[y] = 0;
	}

	int16 x;
	uint16 y = SEARCH_BOT;
	while (y >= g_search_top)
	{
		// 动态阈值，近处阈值大，远处阈值小
		diff_thres = (g_differ_threshold1 - g_differ_threshold2) * (y - SEARCH_TOP) / (SEARCH_BOT - SEARCH_TOP) + g_differ_threshold2;
		// 如果上一行未找到，则进行全白行判断，如果是全白，则跳过
		if (!find_flag && thres_table[g_image[y][SEARCH_LEFT]] && thres_table[g_image[y][x_conditon_left]])
		{
			g_left_line[y] = 0;
			y -= SEARCH_STEP_Y;
			continue;
		}
		// 否则进行搜索
		else
		{
			times++;
			// 第一次寻找根据之前的结果
			if (times == 1)
			{
				x = last_x + margin;
				end_x = SEARCH_LEFT;
			}
			// 第二次寻找扩大范围
			else
			{
				x = g_whiteline_x;
				end_x = last_x + margin + 1;
			}
			// 防止 x 越界
			if (x > SEARCH_RIGHT)
			{
				x = SEARCH_RIGHT;
			}
			// 清除寻找结果
			find_flag = 0x0000;
			// 开始搜索
			for (; x >= end_x; x -= SEARCH_STEP_X)
			{
				if (thres_table[g_image[y][x + 1]] && !thres_table[g_image[y][x]] && !thres_table[g_image[y][x - 1]])
					// if (FAST_DIFFER((int16)g_image[y][x + 1], (int16)g_image[y][x]) > diff_thres)
				{
					g_left_line[y] = x;
					last_x = x;
					find_flag = 0xffff;
					count++;
					times = 0;
					y -= SEARCH_STEP_Y;
					break;
				}
				else
				{
					continue;
				}
			}
			// 如果两次没找到，搜索下一行
			if (!find_flag && times == 2)
			{
				times = 0;
				try1--;
				y -= SEARCH_STEP_Y;
				if (try1)
				{
					continue;
				}
				else
				{
					break;
				}
			}
		}
	}

	g_left_count = count;
	return count;
}

uint16 image_fastsearch_rightline()
{
	int16 end_x = SEARCH_RIGHT;
	uint16 count = 0;
	uint16 times = 0;
	uint16 try1 = SEARCH_TRY;
	uint16 diff_thres = 0;
	uint16 last_x = g_whiteline_x;
	uint16 find_flag = 0x0000;
	uint16 margin = SEARCH_MARGIN;
	uint8* thres_table = g_thres_table;

	for (uint16 y = SEARCH_BOT; y >= SEARCH_TOP; y -= SEARCH_STEP_Y)
	{
		g_right_line[y] = 0;
	}

	int16 x;
	uint16 y = SEARCH_BOT;
	while (y >= g_search_top)
	{
		diff_thres = (g_differ_threshold1 - g_differ_threshold2) * (y - SEARCH_TOP) / (SEARCH_BOT - SEARCH_TOP) + g_differ_threshold2;
		if (!find_flag && thres_table[g_image[y][SEARCH_RIGHT]] && thres_table[g_image[y][x_conditon_right]])
		{
			g_right_line[y] = 0;
			y -= SEARCH_STEP_Y;
			continue;
		}
		else
		{
			times++;
			if (times == 1)
			{
				x = last_x - margin;
				end_x = SEARCH_RIGHT;
			}
			else
			{
				x = g_whiteline_x;
				end_x = last_x - margin + 1;
			}
			if (x < SEARCH_LEFT)
			{
				x = SEARCH_LEFT;
			}
			find_flag = 0x0000;
			for (; x <= end_x; x += SEARCH_STEP_X)
			{
				if (thres_table[g_image[y][x - 1]] && !thres_table[g_image[y][x]] && !thres_table[g_image[y][x + 1]])
					// if (FAST_DIFFER((int16)g_image[y][x - 1], (int16)g_image[y][x]) > diff_thres)
				{
					g_right_line[y] = x;
					last_x = x;
					find_flag = 0xffff;
					count++;
					times = 0;
					y -= SEARCH_STEP_Y;
					break;
				}
				else
				{
					continue;
				}
			}
			if (!find_flag && times == 2)
			{
				times = 0;
				y -= SEARCH_STEP_Y;
				try1--;
				if (try1)
				{
					continue;
				}
				else
				{
					break;
				}
			}
		}
	}

	g_right_count = count;
	return count;
}



//左右边界相关系数判定
uint16 image_get_correlation(uint16 line_type)
{
#define DOWNSAMPLE_STEP	2

	uint16 count = 0;
	uint16* lines = NULL;
	float* correlation = NULL;
	if (line_type == LINE_LEFT)
	{
		count = g_left_count;
		lines = g_left_line;
		correlation = &g_left_correlation;
	}
	else
	{
		count = g_right_count;
		lines = g_right_line;
		correlation = &g_right_correlation;
	}

	if (count < 5)
	{
		return 0xff;
	}

	// 计算总和
	count = 0;
	uint32 sumy = 0;
	uint32 sumx = 0;
	for (int16 y = SEARCH_BOT; y >= SEARCH_TOP; y -= DOWNSAMPLE_STEP)
	{
		if (lines[y])
		{
			sumx += lines[y];
			sumy += y;
			count++;
		}
	}

	// 计算均值
	int16 avex = sumx / count;
	int16 avey = sumy / count;

	int32 R_x = 0;  // y 方差
	int32 R_y = 0;  // x 方差
	int32 R_xy = 0; // x y 协方差
	for (int16 y = SEARCH_BOT; y >= SEARCH_TOP; y -= DOWNSAMPLE_STEP)
	{
		if (lines[y])
		{
			R_x += (lines[y] - avex) * (lines[y] - avex);
			R_y += (y - avey) * (y - avey);
			R_xy += (lines[y] - avex) * (y - avey);
		}
	}

	*correlation = R_xy / sqrtf(R_x * R_y);

	return *correlation > g_correlation_thres;
}

void image_get_roadwidth()
{
	uint16 y_sum = 0;
	uint16 x_sum = 0;
	uint32 xy_sum = 0;
	uint16 n_sum = 0;
	uint32 x2_sum = 0;

	// 计算直线参数
	for (uint16 y = SEARCH_BOT; y >= SEARCH_TOP; y -= 1)
	{
		if (g_left_line[y] && g_right_line[y])
		{
			x_sum += y;
			y_sum += (g_right_line[y] - g_left_line[y]);
			xy_sum += y * (g_right_line[y] - g_left_line[y]);
			n_sum++;
			x2_sum += y * y;
		}
	}

	float x_mean = x_sum / (float)n_sum;
	float k = (xy_sum - x_mean * y_sum) / (x2_sum - x_sum * x_mean);
	float b = y_sum / (float)n_sum - k * x_mean;

	g_road_width_bias = b / 2;
	g_road_width_ratio = k / 2;
}


