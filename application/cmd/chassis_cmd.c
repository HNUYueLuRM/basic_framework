#include <chassis_board_cmd.h>
#include <pub_sub.h>
void chassis_board_com_lost(void* obj) {
    //暂时没有效用
}
chassis_board_cmd::chassis_board_cmd() : sender([&] {
                                             can_send<chassis_board_send>::can_send_config config;
                                             config.device = &BSP_CanTypeDef::can_devices[1];
                                             config.can_identifier = 0x003;
                                             return config;
                                         }()),
                                         recver([&] {
                                             can_recv<gimbal_board_send>::can_recv_config config;
                                             config.device = &BSP_CanTypeDef::can_devices[1];
                                             config.can_identifier = 0x004;
                                             config.lost_callback = chassis_board_com_lost;
                                             return config;
                                         }()),
                                         board_buzzer([&] {
                                             buzzer::buzzer_config config;
                                             //底盘音乐2
                                             config.music = &buzzer::buzzer_musics[2];
                                             config.pwm_device = &BSP_PWM_Typedef::pwm_ports[PWM_BUZZER_PORT];
                                             return config;
                                         }()) {
    robot_mode = robot_stop;
    robot_ready = 0;
    chassis_upload_data = NULL;
    board_recv = recver.recv_data;
    memset(&chassis_control, 0, sizeof(cmd_chassis));
    memset(&board_send, 0, sizeof(chassis_board_send));
}

void chassis_board_cmd::update() {
    //初始化为RUN
    robot_mode = robot_run;

    //判断板间通信在线
    if (!recver.is_online()) {
        robot_mode = robot_stop;  //板间通信掉线，机器人停止
    }

    //接收底盘回传信息，判断底盘IMU在线且初始化完成
    static subscriber<upload_chassis*> chassis_upload_suber("upload_chassis");
    if (!chassis_upload_suber.empty()) {
        chassis_upload_data = chassis_upload_suber.pop();
        if (chassis_upload_data->chassis_status == module_lost) {  //底盘模块掉线
            robot_mode = robot_stop;
        }
    }

    if (chassis_upload_data == NULL) {  //底盘模块初始化尚未完成，第一次回传数据未收到
        robot_mode = robot_stop;
    } else {
        board_send.gyro_yaw = chassis_upload_data->chassis_imu->euler[2];
    }

    //判断裁判系统是否在线，并处理掉线情况（未实现）
    board_send.heat_limit_remain = 30;  //读取裁判系统
    board_send.bullet_speed_max = 30;   //读取裁判系统

    //判断除了云台板stop之外，都已经上线，说明底盘板初始化完成，进入ready状态
    if (robot_mode == robot_run) {
        if (!robot_ready) {
            robot_ready = 1;
            board_buzzer.start();  //播放音乐
        }
        board_send.chassis_board_status = module_run;
    } else {
        board_send.chassis_board_status = module_lost;
    }

    //云台板进入stop模式
    if (board_recv->robot_mode == robot_stop) {
        robot_mode = robot_stop;
    }

    if (robot_mode == robot_stop) {
        // STOP模式
        chassis_control.mode = chassis_stop;
    } else {
        // RUN模式
        // 底盘控制指令
        chassis_control.mode = board_recv->chassis_mode;
        chassis_control.speed = board_recv->chassis_speed;
        chassis_control.power.power_buffer = 0;  //应该由裁判系统获得，未实现
        chassis_control.power.power_limit = 50;  //应该由裁判系统获得，未实现
    }

    //发布指令
    static publisher<cmd_chassis*> chassis_puber("cmd_chassis");
    chassis_puber.push(&chassis_control);

    //板间通信
    sender.send(board_send);
}