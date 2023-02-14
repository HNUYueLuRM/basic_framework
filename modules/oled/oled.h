/**
 * @file oled.h
 * @author your name (you@domain.com)
 * @brief 待重构实现
 * @version 0.1
 * @date 2023-02-14
 * @todo 请重构show string/init/clean/update buffer等的实现
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#ifndef OLED_H
#define OLED_H
#include <stdint.h>

// the I2C address of oled
#define OLED_I2C_ADDRESS 0x78

// the resolution of oled   128*64
#define MAX_COLUMN 128
#define MAX_ROW 64

#define X_WIDTH MAX_COLUMN
#define Y_WIDTH MAX_ROW

#define OLED_CMD 0x00
#define OLED_DATA 0x01

#define CHAR_SIZE_WIDTH 6
#define CHAR_SIZE_HIGHT 12

typedef enum
{
  PEN_CLEAR = 0x00,
  PEN_WRITE = 0x01,
  PEN_INVERSION = 0x02,
} pen_typedef;

/**
 * @brief          初始化OLED模块，
 * @param[in]      none
 * @retval         none
 */
extern void OLED_init(void);

/**
 * @brief          打开OLED显示
 * @param[in]      none
 * @retval         none
 */
extern void OLED_display_on(void);

/**
 * @brief          关闭OLED显示
 * @param[in]      none
 * @retval         none
 */
extern void OLED_display_off(void);

/**
  * @brief          操作GRAM内存(128*8char数组)
  * @param[in]      pen: 操作类型.
                    PEN_CLEAR: 设置为0x00
                    PEN_WRITE: 设置为0xff
                    PEN_INVERSION: 按位取反
  * @retval         none
  */
extern void OLED_operate_gram(pen_typedef pen);

/**
 * @brief          设置光标起点(x,y)
 * @param[in]      x:x轴, 从 0 到 127
 * @param[in]      y:y轴, 从 0 到 7
 * @retval         none
 */
extern void OLED_set_pos(uint8_t x, uint8_t y);

/**
  * @brief          操作GRAM中的一个位，相当于操作屏幕的一个点
  * @param[in]      x:x轴,  [0,X_WIDTH-1]
  * @param[in]      y:y轴,  [0,Y_WIDTH-1]
  * @param[in]      pen: 操作类型,
                        PEN_CLEAR: 设置 (x,y) 点为 0
                        PEN_WRITE: 设置 (x,y) 点为 1
                        PEN_INVERSION: (x,y) 值反转
  * @retval         none
  */
extern void OLED_draw_point(int8_t x, int8_t y, pen_typedef pen);

/**
 * @brief          画一条直线，从(x1,y1)到(x2,y2)
 * @param[in]      x1: 起点
 * @param[in]      y1: 起点
 * @param[in]      x2: 终点
 * @param[in]      y2: 终点
 * @param[in]      pen: 操作类型,PEN_CLEAR,PEN_WRITE,PEN_INVERSION.
 * @retval         none
 */
extern void OLED_draw_line(uint8_t x1, uint8_t y1, uint8_t x2, uint8_t y2, pen_typedef pen);

/**
 * @brief          显示一个字符
 * @param[in]      row: 字符的开始行
 * @param[in]      col: 字符的开始列
 * @param[in]      chr: 字符
 * @retval         none
 */
extern void OLED_show_char(uint8_t row, uint8_t col, uint8_t chr);

/**
 * @brief          显示一个字符串
 * @param[in]      row: 字符串的开始行
 * @param[in]      col: 字符串的开始列
 * @param[in]      chr: 字符串
 * @retval         none
 */
extern void OLED_show_string(uint8_t row, uint8_t col, uint8_t *chr);

/**
 * @brief          格式输出
 * @param[in]      row: 开始列，0 <= row <= 4;
 * @param[in]      col: 开始行， 0 <= col <= 20;
 * @param[in]      *fmt:格式化输出字符串
 * @note           如果字符串长度大于一行，额外的字符会换行
 * @retval         none
 */
extern void OLED_printf(uint8_t row, uint8_t col, const char *fmt, ...);


/**
 * @brief          发送数据到OLED的GRAM
 * @param[in]      none
 * @retval         none
 */
extern void OLED_refresh_gram(void);


/**
 * @brief          显示RM的LOGO
 * @param[in]      none
 * @retval         none
 */
extern void OLED_LOGO(void);
#endif
