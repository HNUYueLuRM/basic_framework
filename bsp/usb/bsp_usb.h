#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_if.h"

typedef enum
{
    USB_CONNECTION_RAWDATA = 0,
    USB_CONNECTION_VOFA,
    USB_CONNECTION_SEASKY,
} USB_Connection_Type_e; // 选择usb连接模式,默认为原始数据
// 方便调试,也可以不用

/* @note 虚拟串口的波特率/校验位/数据位等动态可变,取决于上位机的设定 */
/* 使用时不需要关心这些设置(作为从机) */

uint8_t *USBInit(); // bsp初始化时调用会重新枚举设备

void USBTransmit(uint8_t *buffer, uint16_t len); // 通过usb发送数据