# ins_task

<p align='right'>neozng1@hnu.edu.cn</p>

## 硬触发流程

![image-20221113212706633](.assets\image-20221113212706633.png)

`times%10` 是固定相机的采集频率为100hz，请根据视觉算法实际能达到的最大帧率调整。

## 算法解析

介绍EKF四元数姿态解算的教程在:[四元数EKF姿态更新算法](https://zhuanlan.zhihu.com/p/454155643)

## 模块移植

由于历史遗留问题,IMU模块耦合程度高.后续实现BSP_SPI,将bmi088 middleware删除.仅保留BMI088读取的协议和寄存器定义等,单独实现IMU模块.
> 移植已经完成,请转而使用module/BMI088的模块. 当前文件夹将在beta1.2停止支持, 1.5之后删除. INS_Task届时会被放到algorithm中,以提供对不同IMU的兼容.
