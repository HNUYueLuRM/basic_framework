#ifndef _BSP_LOG_H
#define _BSP_LOG_H

void BSP_Log_Init();

int printf_log(const char *fmt, ...);

void Float2Str(char *str, float va);

#endif