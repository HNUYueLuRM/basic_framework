# gimbal



## 工作流程

初始化pitch和yaw电机以及一个imu。订阅gimbal_cmd消息（来自robot_cmd）并发布gimbal_feed话题。

1. 从消息中心获取gimbal_cmd话题的消息
2. 根据消息中的控制模式进行模式切换，如果急停则关闭所有电机
3. 由设定的模式，进行电机反馈数据来源的切换，修改反馈数据指针，设置前馈控制数据指针等。
4. 设置反馈数据，包括yaw电机的绝对角度和imu数据
5. 推送反馈数据到gimbal_feed话题下