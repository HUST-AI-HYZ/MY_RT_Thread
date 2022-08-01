/*
 * @Author: hyz
 * @Date: 2022-07-29 11:28:00
 * @LastEditors: HUST-AI-HYZ
 * @LastEditTime: 2022-07-30 21:49:30
 * @Description: file content
 */ 
//#include <rtthread.h>
//include <rtdevice.h>
#include "common.h"
#include "image.h"
#include "imparam.h"
#include "parameter.h"

void linesearch_sample_entry(void *p)
{
    //获取到一幅灰度图像，把数组载入 g_image[MT9V03X_H][MT9V03X_W]便可
	//get_image(g_image); 
    image_fast_otsu(); // OTSU计算最佳阈值
	image_update_thresvalue(); // 更新二值化查找表
	image_find_whiteline(); // 寻找最长白列
	image_fastsearch_leftline(); // 搜左线
	image_fastsearch_rightline(); // 搜右线
    image_find_midline(1); // 计算中线
	image_update_err_weight(); // 更新误差权重
	image_calcu_err(); // 计算误差
	image_debug(); // 生成图像调试画面


int linesearch_sample_entry_init(void)
{
    rt_thread_t t = rt_thread_create(
        "linesearch", linesearch_sample_entry, RT_NULL, 
        2048, 20, 10
    );
    if (t == RT_NULL)
    {
        rt_kprintf("Failde to create linesearch info procees thread.\r\n");
        return RT_ERROR;
    }
    if (rt_thread_startup(t) != RT_EOK)
    {
        rt_kprintf("Failde to startup linesearch info procees thread.\r\n");
        rt_thread_delete(t);
        return RT_ERROR;
    }
    return RT_EOK;
}
INIT_APP_EXPORT(linesearch_sample_entry_init);
