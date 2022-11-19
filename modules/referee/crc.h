#ifndef __CRC_H_
#define __CRC_H_

#include <stdint-gcc.h>

#define TRUE 1
#define FALSE 0
// CRC8
void Append_CRC8_Check_Sum(uint8_t *pchMessage, uint16_t dwLength);
uint32_t Verify_CRC8_Check_Sum(uint8_t *pchMessage, uint16_t dwLength);
uint8_t Get_CRC8_Check_Sum(uint8_t *pchMessage, uint16_t dwLength, uint8_t ucCRC8);

// CRC16
void Append_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);
uint32_t Verify_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength);
uint16_t Get_CRC16_Check_Sum(uint8_t *pchMessage, uint32_t dwLength, uint16_t wCRC);

#endif
