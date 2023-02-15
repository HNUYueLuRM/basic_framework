/**
 * @file led_task.c
 * @author DJI RM
 * @author modified by NeoZeng neozng1@hnu.edu.cn
 * @brief 流水灯效
 * @version 0.1
 * @date 2022-11-30
 *
 * @copyright Copyright (c) 2022
 *
 */

#warning this is a legacy support file, please build the new 'led' module as soon as possible

#include "led_task.h"
#include <stdint.h>
#include "bsp_led.h"
#include "main.h"

#define RGB_FLOW_COLOR_CHANGE_TIME 1000
#define RGB_FLOW_COLOR_LENGHT 6

// 蓝 -> 绿(灭) -> 红 -> 蓝(灭) -> 绿 -> 红(灭) -> 蓝
uint32_t RGB_flow_color[RGB_FLOW_COLOR_LENGHT + 1] = {0xFF0000FF, 0x0000FF00, 0xFFFF0000, 0x000000FF, 0xFF00FF00, 0x00FF0000, 0xFF0000FF};

void led_RGB_flow_task()
{
	static float delta_alpha, delta_red, delta_green, delta_blue;
	static float alpha, red, green, blue;
	static uint32_t aRGB;

	for (size_t i = 0; i < RGB_FLOW_COLOR_LENGHT; ++i)
	{
		alpha = (RGB_flow_color[i] & 0xFF000000) >> 24;
		red = ((RGB_flow_color[i] & 0x00FF0000) >> 16);
		green = ((RGB_flow_color[i] & 0x0000FF00) >> 8);
		blue = ((RGB_flow_color[i] & 0x000000FF) >> 0);

		delta_alpha = (float)((RGB_flow_color[i + 1] & 0xFF000000) >> 24) - (float)((RGB_flow_color[i] & 0xFF000000) >> 24);
		delta_red = (float)((RGB_flow_color[i + 1] & 0x00FF0000) >> 16) - (float)((RGB_flow_color[i] & 0x00FF0000) >> 16);
		delta_green = (float)((RGB_flow_color[i + 1] & 0x0000FF00) >> 8) - (float)((RGB_flow_color[i] & 0x0000FF00) >> 8);
		delta_blue = (float)((RGB_flow_color[i + 1] & 0x000000FF) >> 0) - (float)((RGB_flow_color[i] & 0x000000FF) >> 0);

		delta_alpha /= RGB_FLOW_COLOR_CHANGE_TIME;
		delta_red /= RGB_FLOW_COLOR_CHANGE_TIME;
		delta_green /= RGB_FLOW_COLOR_CHANGE_TIME;
		delta_blue /= RGB_FLOW_COLOR_CHANGE_TIME;
		for (size_t j = 0; j < RGB_FLOW_COLOR_CHANGE_TIME; ++j)
		{
			alpha += delta_alpha;
			red += delta_red;
			green += delta_green;
			blue += delta_blue;

			aRGB = ((uint32_t)(alpha)) << 24 | ((uint32_t)(red)) << 16 | ((uint32_t)(green)) << 8 | ((uint32_t)(blue)) << 0;
			FlowRGBShow(aRGB);
		}
	}
}
