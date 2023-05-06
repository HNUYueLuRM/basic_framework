#ifndef __TFMINIPLUS_H__
#define __TFMINIPLUS_H__

#include "stdint.h"
#include "bsp_iic.h"

typedef struct
{
	IICInstance *iic;
	uint8_t Mode;	   //= buf[6];
	uint16_t Dist;	   //= buf[2] | (buf[3] << 8);
	uint32_t Strength; //= buf[4] | (buf[5] << 8);
	uint8_t buf[9];	
} TFMiniInstance;

TFMiniInstance *TFMiniRegister(I2C_HandleTypeDef *hi2c);

float GetDistance(TFMiniInstance *tfmini);

#endif
