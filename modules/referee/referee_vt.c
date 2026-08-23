#include "referee_vt.h"
#include "string.h"
#include "crc_ref.h"
#include "bsp_usart.h"
#include "task.h"
#include "daemon.h"
#include "bsp_log.h"
#include "cmsis_os.h"
#define RE_VT_SUFFER_SIZE 255U //计算的其实60U就够装两个信息了，怕不够先定100

static USARTInstance *referee_vt_usart_instance;
static DaemonInstance *referee_vt_daemon;
static referee_vt_info_t referee_vt_info;  //这三个基本是模仿rm_referee.c,常规链路和图传链路的信息分开
static void JudgeVtData(uint8_t *vt_buff)//都static了就不考虑复用性了，还是得考虑单板的车同时跑两个链路
{
	uint16_t judge_length; // 统计一帧数据长度
	if (vt_buff == NULL)	   // 空数据包，则不作任何处理
    {
		return;
    }
	// 写入帧头数据(5-byte),用于判断是否开始存储裁判数据
	memcpy(&referee_vt_info.FrameHeader, vt_buff, LEN_HEADER);

	// 判断帧头数据(0)是否为0xA5
	if (vt_buff[SOF] == REFEREE_SOF)
	{
		// 帧头CRC8校验
		if (Verify_CRC8_Check_Sum(vt_buff, LEN_HEADER) == TRUE)
		{
			// 统计一帧数据长度(byte),用于CR16校验
			judge_length = vt_buff[DATA_LENGTH] + LEN_HEADER + LEN_CMDID + LEN_TAIL;
			// 帧尾CRC16校验
			if (Verify_CRC16_Check_Sum(vt_buff, judge_length) == TRUE)
			{
				// 2个8位拼成16位int
				referee_vt_info.CmdID = (vt_buff[6] << 8 | vt_buff[5]);
				// 解析数据命令码,将数据拷贝到相应结构体中(注意拷贝数据的长度)
				// 第8个字节开始才是数据 data=7
				switch (referee_vt_info.CmdID)// todo:加入自定义客户端相关的解析，但考虑新赛季裁判系统可能大改先暂时放置
				{
				case ID_custom2robot: // 0x0302
					memcpy(&referee_vt_info.Custom2Robot, (vt_buff + DATA_Offset), LEN_custom2robot_data);
					break;
				case ID_selfclient2robot: // 0x0311
					memcpy(&referee_vt_info.SelfClient2Robot,(vt_buff + DATA_Offset), LEN_selfclient2robot);
					break;
				}
			}
		}
		// 首地址加帧长度,指向CRC16下一字节,用来判断是否为0xA5,从而判断一个数据包是否有多帧数据
		if (*(vt_buff + sizeof(xFrameHeader) + LEN_CMDID + referee_vt_info.FrameHeader.DataLength + LEN_TAIL) == 0xA5)
		{ // 如果一个数据包出现了多帧数据,则再次调用解析函数,直到所有数据包解析完毕
			JudgeVtData(vt_buff + sizeof(xFrameHeader) + LEN_CMDID + referee_vt_info.FrameHeader.DataLength + LEN_TAIL);
		}
	}

}
//图传链路需要发送一些东西
/**
 * @brief 裁判系统图传链路数据发送函数
 * @param
 */
void VT_SelfClient_RefereeSend(uint8_t *send, uint16_t tx_len)
{
	USARTSend(referee_vt_usart_instance, send, tx_len, USART_TRANSFER_DMA);
	osDelay(20);
}

void VT_Custom_RefereeSend(uint8_t *send, uint16_t tx_len)
{
	USARTSend(referee_vt_usart_instance, send, tx_len, USART_TRANSFER_DMA);
	osDelay(100);
}

static void RefereeVtCallback()
{
    DaemonReload(referee_vt_daemon);
    JudgeVtData(referee_vt_usart_instance->recv_buff);
}
// 裁判系统丢失处理
static void RefereeVtLost(void *arg)
{
    USARTServiceInit(referee_vt_usart_instance);
    LOGWARNING("referee vt is jj");
}

referee_vt_info_t *RefereeVtInit(UART_HandleTypeDef *referee_vt_usart_handle)
{
  USART_Init_Config_s conf;
  conf.module_callback = RefereeVtCallback;
  conf.usart_handle = referee_vt_usart_handle;
  conf.recv_buff_size = RE_VT_SUFFER_SIZE;
  referee_vt_usart_instance = USARTRegister(&conf);
  Daemon_Init_Config_s daemon_conf = {
    .callback = RefereeVtLost,
    .owner_id = referee_vt_usart_instance,
    .reload_count = 30,
  };
  referee_vt_daemon = DaemonRegister(&daemon_conf);
    return &referee_vt_info;
}

