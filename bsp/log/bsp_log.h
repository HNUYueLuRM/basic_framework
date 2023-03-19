#ifndef _BSP_LOG_H
#define _BSP_LOG_H

#include "SEGGER_RTT.h"
#include "SEGGER_RTT_Conf.h"
#include <stdio.h>

#define BUFFER_INDEX 0



/**
 * @brief 日志系统初始化
 * 
 */
void BSPLogInit();

/**
 * @brief 日志功能原型,供下面的LOGI,LOGW,LOGE等使用
 * 
 */
#define LOG_PROTO(type,color,format,...)            \
        SEGGER_RTT_printf(BUFFER_INDEX,"  %s%s"format"\r\n%s", \
                          color,                    \
                          type,                     \
                          ##__VA_ARGS__,            \
                          RTT_CTRL_RESET)

/*------下面是日志输出的接口--------*/

/* 清屏 */
#define LOG_CLEAR() SEGGER_RTT_WriteString(0, "  "RTT_CTRL_CLEAR)

/* 无颜色日志输出 */
#define LOG(format,...) LOG_PROTO("","",format,##__VA_ARGS__)

/* 有颜色格式日志输出,建议使用这些宏来输出日志 */
// information level
#define LOGINFO(format,...) LOG_PROTO("I", RTT_CTRL_TEXT_BRIGHT_GREEN , format, ##__VA_ARGS__)
// warning level
#define LOGWARNING(format,...) LOG_PROTO("W", RTT_CTRL_TEXT_BRIGHT_YELLOW, format, ##__VA_ARGS__)
// error level
#define LOGERROR(format,...) LOG_PROTO("E", RTT_CTRL_TEXT_BRIGHT_RED   , format, ##__VA_ARGS__)

/**
 * @brief 通过segger RTT打印日志,支持格式化输出,格式化输出的实现参考printf.
 * 
 * @param fmt 格式字符串
 * @param ... 参数列表
 * @return int 打印的log字符数
 */
int PrintLog(const char *fmt, ...);

/**
 * @brief 利用sprintf(),将float转换为字符串进行打印
 * 
 * @param str 转换后的字符串
 * @param va 待转换的float
 */
void Float2Str(char *str, float va);

#endif