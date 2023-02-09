/**
 * @file bsp_usb.h
 * @author your name (you@domain.com)
 * @brief 提供对usb vpc(virtal com port)的操作接口,hid和msf考虑后续添加
 * @todo 与usart的接口统一完成兼容
 * @attention 这一版修改了usbd_cdc_if.c中的CDC_Receive_FS函数,cube生成后会被覆盖.后续需要由usbcdciftemplate创建一套自己的模板
 * @version 0.1
 * @date 2023-02-09
 * 
 * @copyright Copyright (c) 2023
 * 
 */

#include "usb_device.h"
#include "usbd_cdc.h"
#include "usbd_conf.h"
#include "usbd_desc.h"
#include "usbd_cdc_if.h"

/* @note 虚拟串口的波特率/校验位/数据位等动态可变,取决于上位机的设定 */
/* 使用时不需要关心这些设置(作为从机) */

uint8_t *USBInit(); // bsp初始化时调用会重新枚举设备

void USBTransmit(uint8_t *buffer, uint16_t len); // 通过usb发送数据
