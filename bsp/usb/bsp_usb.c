/**
 * @file bsp_usb.c
 * @author your name (you@domain.com)
 * @brief usb是单例bsp,因此不保存实例
 * @version 0.1
 * @date 2023-02-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "bsp_usb.h"

static uint8_t *bsp_usb_rx_buffer; // 接收到的数据会被放在这里,buffer size为2048
// 注意usb单个数据包(Full speed模式下)最大为64byte,超出可能会出现丢包情况

// 这是传输完成的回调函数,在usbd_cdc_if.c中被调用
__weak void USBTransmitCpltCallback(uint32_t len)
{
    // 本次发送的数据长度
    UNUSED(len);
    // 传输完成会调用此函数,to do something
}

// 这是接收回调函数
__weak void USBReceiveCpltCallback(uint32_t len)
{
    // 本次接收的数据长度
    UNUSED(len);
    // 传输完成会调用此函数,to do something
}

uint8_t *USBInit()
{
    // 上电后重新枚举usb设备
    USBTransmit((uint8_t *)"USB DEVICE READY", sizeof("USB DEVICE READY")); // 发送初始化完成信息
    bsp_usb_rx_buffer = CDCInitRxbufferNcallback(USBTransmitCpltCallback,USBReceiveCpltCallback);  // 获取接收数据指针
    // usb的接收回调函数会在这里被设置,并将数据保存在bsp_usb_rx_buffer中
    return bsp_usb_rx_buffer;
}

void USBTransmit(uint8_t *buffer, uint16_t len)
{
    CDC_Transmit_FS(buffer, len); // 发送
}
