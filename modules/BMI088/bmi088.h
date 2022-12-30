#include "bsp_spi.h"

/* BMI088实例结构体定义 */
typedef struct
{
    SPIInstance *spi_gyro; 
    SPIInstance *spi_acc;


} BMI088Instance;

/* BMI088初始化配置 */
typedef struct
{

}BMI088_Init_Config_s;

