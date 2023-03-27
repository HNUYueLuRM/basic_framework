#include "ps_handle.h"
uint8_t PS2_RawData[9] = {0};
PS2_Instance PS2_Data = {0};
void PS2_CS(uint8_t Val)
{
    if (Val)
        HAL_GPIO_WritePin(PS2_CS_GPIOx, PS2_CS_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(PS2_CS_GPIOx, PS2_CS_Pin, GPIO_PIN_RESET);
}
void PS2_CLK(uint8_t Val)
{
    if (Val)
        HAL_GPIO_WritePin(PS2_CLK_GPIOx, PS2_CLK_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(PS2_CLK_GPIOx, PS2_CLK_Pin, GPIO_PIN_RESET);
}
void PS2_DO(uint8_t Val)
{
    if (Val)
        HAL_GPIO_WritePin(PS2_DO_GPIOx, PS2_DO_Pin, GPIO_PIN_SET);
    else
        HAL_GPIO_WritePin(PS2_DO_GPIOx, PS2_DO_Pin, GPIO_PIN_RESET);
}
uint8_t PS2_Read_DI()
{
    return HAL_GPIO_ReadPin(PS2_DI_GPIOx, PS2_DI_Pin);
}
void PS2_Delay()
{
    for (int i = 0; i < 0xBf; i++)
       __NOP();
}
uint8_t PS2_ReadWrite_Byte(uint8_t TxData)
{
    uint8_t TX = TxData;
    uint8_t RX = 0;
    for (int i = 0; i < 8; i++)
    {
        if (TX & 0x01)
            PS2_DO(1);
        else
            PS2_DO(0);
        TX >>= 1;
        PS2_CLK(1);
        PS2_Delay();
        PS2_CLK(0);
        RX >>= 1;
        RX |= (PS2_Read_DI() << 7);
        PS2_Delay();
        PS2_CLK(1);
        PS2_Delay();
    }
    return RX;
}

static void PS2_Decode()
{
    if (PS2_RawData[2] == 0x5A)
    {
        PS2_Data.Key_Select = (~PS2_RawData[3] >> 0) & 0x01; //选择键
        PS2_Data.Key_Start = (~PS2_RawData[3] >> 3) & 0x01;  //开始键

        //左侧按键
        PS2_Data.Key_L_Up = (~PS2_RawData[3] >> 4) & 0x01;
        PS2_Data.Key_L_Right = (~PS2_RawData[3] >> 5) & 0x01;
        PS2_Data.Key_L_Down = (~PS2_RawData[3] >> 6) & 0x01;
        PS2_Data.Key_L_Left = (~PS2_RawData[3] >> 7) & 0x01;

        //后侧按键
        PS2_Data.Key_L2 = (~PS2_RawData[4] >> 0) & 0x01;
        PS2_Data.Key_R2 = (~PS2_RawData[4] >> 1) & 0x01;
        PS2_Data.Key_L1 = (~PS2_RawData[4] >> 2) & 0x01;
        PS2_Data.Key_R1 = (~PS2_RawData[4] >> 3) & 0x01;

        //右侧按键
        PS2_Data.Key_R_Up = (~PS2_RawData[4] >> 4) & 0x01;
        PS2_Data.Key_R_Right = (~PS2_RawData[4] >> 5) & 0x01;
        PS2_Data.Key_R_Down = (~PS2_RawData[4] >> 6) & 0x01;
        PS2_Data.Key_R_Left = (~PS2_RawData[4] >> 7) & 0x01;

        if (PS2_RawData[1] == 0x41)
        { //无灯模式(摇杆值八向)
            PS2_Data.Rocker_LX = 127 * (PS2_Data.Key_L_Right - PS2_Data.Key_L_Left);
            PS2_Data.Rocker_LY = 127 * (PS2_Data.Key_L_Up - PS2_Data.Key_L_Down);

            PS2_Data.Rocker_RX = 127 * (PS2_Data.Key_R_Right - PS2_Data.Key_R_Left);
            PS2_Data.Rocker_RY = 127 * (PS2_Data.Key_R_Up - PS2_Data.Key_R_Down);
        }
        else if (PS2_RawData[1] == 0x73)
        { //红灯模式(摇杆值模拟)

            //摇杆按键
            PS2_Data.Key_Rocker_Left = (~PS2_RawData[3] >> 1) & 0x01;
            PS2_Data.Key_Rocker_Right = (~PS2_RawData[3] >> 2) & 0x01;

            //摇杆值
            PS2_Data.Rocker_LX = PS2_RawData[7] - 0x80;
            PS2_Data.Rocker_LY = -1 - (PS2_RawData[8] - 0x80);
            PS2_Data.Rocker_RX = PS2_RawData[5] - 0x80;
            PS2_Data.Rocker_RY = -1 - (PS2_RawData[6] - 0x80);
        }
    }
}
void PS2_Read_Data(void)
{
    PS2_CS(0);
    PS2_RawData[0] = PS2_ReadWrite_Byte(0x01); // 0
    PS2_RawData[1] = PS2_ReadWrite_Byte(0x42); // 1
    for (int i = 2; i < 9; i++)
        PS2_RawData[i] = PS2_ReadWrite_Byte(0xff);
    PS2_CS(1);
    PS2_Decode();
}

