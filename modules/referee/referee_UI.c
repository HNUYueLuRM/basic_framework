// /**
//   * @brief  上传自定义数据
//   * @param  void
//   * @retval void
//   * @attention  数据打包,打包完成后通过串口发送到裁判系统
//   */
// #define send_max_len     200
// unsigned char CliendTxBuffer[send_max_len];
// void JUDGE_Show_Data(void)
// {
// 	static u8 datalength,i;
// 	uint8_t judge_led = 0xff;//初始化led为全绿
// 	static uint8_t auto_led_time = 0;
// 	static uint8_t buff_led_time = 0;
	
// 	determine_ID();//判断发送者ID和其对应的客户端ID
	
// 	ShowData.txFrameHeader.SOF = 0xA5;
// 	ShowData.txFrameHeader.DataLength = sizeof(ext_student_interactive_header_data_t) + sizeof(client_custom_data_t);
// 	ShowData.txFrameHeader.Seq = 0;
// 	memcpy(CliendTxBuffer, &ShowData.txFrameHeader, sizeof(xFrameHeader));//写入帧头数据
// 	Append_CRC8_Check_Sum(CliendTxBuffer, sizeof(xFrameHeader));//写入帧头CRC8校验码
	
// 	ShowData.CmdID = 0x0301;
	
// 	ShowData.dataFrameHeader.data_cmd_id = 0xD180;//发给客户端的cmd,官方固定
// 	//ID已经是自动读取的了
// 	ShowData.dataFrameHeader.send_ID 	 = Judge_Self_ID;//发送者的ID
// 	ShowData.dataFrameHeader.receiver_ID = Judge_SelfClient_ID;//客户端的ID，只能为发送者机器人对应的客户端
	
// 	/*- 自定义内容 -*/
// 	ShowData.clientData.data1 = (float)Capvoltage_Percent();//电容剩余电量
// 	ShowData.clientData.data2 = (float)Base_Angle_Measure();//吊射角度测
// 	ShowData.clientData.data3 = GIMBAL_PITCH_Judge_Angle();//云台抬头角度
// 	ShowData.clientData.masks = judge_led;//0~5位0红灯,1绿灯
	
// 	//打包写入数据段
// 	memcpy(	
// 			CliendTxBuffer + 5, 
// 			(uint8_t*)&ShowData.CmdID, 
// 			(sizeof(ShowData.CmdID)+ sizeof(ShowData.dataFrameHeader)+ sizeof(ShowData.clientData))
// 		  );			
			
// 	Append_CRC16_Check_Sum(CliendTxBuffer,sizeof(ShowData));//写入数据段CRC16校验码	

// 	datalength = sizeof(ShowData); 
// 	for(i = 0;i < datalength;i++)
// 	{
// 		USART_SendData(UART5,(uint16_t)CliendTxBuffer[i]);
// 		while(USART_GetFlagStatus(UART5,USART_FLAG_TC)==RESET);
// 	}	 
// }




// /**
//   * @brief  判断自己红蓝方
//   * @param  void
//   * @retval RED   BLUE
//   * @attention  数据打包,打包完成后通过串口发送到裁判系统
//   */
// bool Color;
// bool is_red_or_blue(void)
// {
// 	Judge_Self_ID = GameRobotStat.robot_id;//读取当前机器人ID
	
// 	if(GameRobotStat.robot_id > 10)
// 	{
// 		return BLUE;
// 	}
// 	else 
// 	{
// 		return RED;
// 	}
// }

// /**
//   * @brief  判断自身ID，选择客户端ID
//   * @param  void
//   * @retval RED   BLUE
//   * @attention  数据打包,打包完成后通过串口发送到裁判系统
//   */
// void determine_ID(void)
// {
// 	Color = is_red_or_blue();
// 	if(Color == BLUE)
// 	{
// 		Judge_SelfClient_ID = 0x0110 + (Judge_Self_ID-10);//计算客户端ID
// 	}
// 	else if(Color == RED)
// 	{
// 		Judge_SelfClient_ID = 0x0100 + Judge_Self_ID;//计算客户端ID
// 	}
// }          