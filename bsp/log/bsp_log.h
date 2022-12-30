#ifndef _BSP_LOG_H
#define _BSP_LOG_H

void BSPLogInit();

int PrintLog(const char *fmt, ...);

void Float2Str(char *str, float va);

#endif