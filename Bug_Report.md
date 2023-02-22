# 异常报告

使用中遇到的bug和错误放在此处。参照下列格式：

## 标题用简短的一句话描述

### 出现问题的application/module/bsp

描述你的使用方法，应该贴上图片或代码块，以及硬件连线等

### 尝试解决的方案

你的尝试，以及猜测可能的错误

### 如何复现问题

问题能否稳定复现？描述复现方法等

### 紧急程度

这里用⭐表示。最大5颗⭐。

如果不修复，会有何种其他牵连情况发生？

---

不同的问题用 --- 分隔开

你还可以使用Stepsize插件在代码出现问题（可能出现问题）的地方添加issues并详尽描述。或在gitee上增加issues。

当然，最快的方法是在群里提问。

## 使用LK电机并挂载在hcan2上时会出现HardFault

使用MF9025v2电机,并将其配置在CAN2上。经过一次LKMotorControl，第二次进入时hcan->instance会在HAL_CAN_Add_Tx_Message()结束时被未知的语句修改成奇怪的值，造成HardFault

### 尝试解决的方案

单步调试无果，在HAL_CAN_Add_Tx_Message()返回的那一步hcan->instance会莫名其妙变成0，hcan2也会被修改到一个0x8000xxx的地址上(hcan是HAL库自定的全局变量)

### 如何复现问题

使用LK电机并将其挂载在CAN2上，连接电机后直接运行。在第二次进入MotorTAsk中的LKMotorControl时，于检查空闲CAN邮箱时，由于hcan2被修改，访问CAN2外设状态时会访问野指针导致HardFault。

### 紧急程度

⭐⭐⭐⭐⭐