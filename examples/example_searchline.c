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
    //��ȡ��һ���Ҷ�ͼ�񣬰��������� g_image[MT9V03X_H][MT9V03X_W]���
	//get_image(g_image); 
    image_fast_otsu(); // OTSU���������ֵ
	image_update_thresvalue(); // ���¶�ֵ�����ұ�
	image_find_whiteline(); // Ѱ�������
	image_fastsearch_leftline(); // ������
	image_fastsearch_rightline(); // ������
    image_find_midline(1); // ��������
	image_update_err_weight(); // �������Ȩ��
	image_calcu_err(); // �������
	image_debug(); // ����ͼ����Ի���


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
