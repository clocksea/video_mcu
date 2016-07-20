/********************************************************************
版权所有	：Copyright (C) 2011 承联通信
文件名字	：ccu串口命令解析
文件功能	：ccu串口命令解析
创建日期	：2011-06-09
创建人		：ZhongH
修改标识	：
修改描述	：
*********************************************************************/
#include "includes.h"

#define CMD_TYPE_SEND_MIDLEV      0x01                       /*配置发射信号中心电平*/
#define CMD_TYPE_SEND_RANGE       0x02                       /*配置发射信号幅度*/
#define CMD_TYPE_BALANCE_MOD1   0x03                       /*配置电台平衡mod1*/
#define CMD_TYPE_BALANCE_MOD2   0x04                       /*配置电台平衡mod2*/
#define CMD_TYPE_SEND_REF_OSCI_LEV         0x05         /*配置电台发射晶振参数电平*/
#define CMD_TYPE_SEND_REF_POWER_LEV      0x06        /*配置电台发射功率参数电平*/
#define CMD_TYPE_RECEIVE_NARROW_BAND    0x07        /*配置电台接受宽窄带*/
#define CMD_TYPE_PRINT_ALL_SEND_FREG      0x08        /*打印当前发射频率的所有参数*/
#define CMD_TYPE_PRINT_ALL_RECEIVE_FREG 0x09         /*打印当前接受的所有频率*/
#define CMD_TYPE_PRINT_ALL_PARAM  0x0A                   /*打印当前所有参数*/
#define CMD_TYPE_SET_RANGE_FREG   0x0B                    /*设置频率范围*/
#define CMD_TYPE_SAVE_CURRENT_PUB_PARAM      0x0C/*保存当前公共参数*/
#define CMD_TYPE_SAVE_CURRENT_SEND_FREG      0x0D /*保存当前发射参数*/
#define CMD_TYPE_SAVE_CURRENT_RECEIVE_FREG 0x0E /*保存当前接受频率参数*/
#define CMD_TYPE_DEL_ASSIGN_SEND_FREG          0x0F  /*删除当前发射频率参数*/
#define CMD_TYPE_TX_RX_POWER                            0x10 /*仅配置电台发射频率，接收频率和发射功率*/
#define CMD_TYPE_SET_PT                                       0x11 /*仅配置电台平衡参数*/
#define CMD_TYPE_READ_MCU_VERSION                  0x12 /*读MCU软件版本*/
#define CMD_TYPE_POTS_QUERY                              0x13 /*读取电台平衡参数*/

#define CMD_TYPE_RECV_RANGE       0x14                       /*配置接收信号幅度*/
#define CMD_TYPE_RECV_MIDLEV      0x15                       /*配置接收信号中心电平*/

#define CMD_TYPE_SET_PLL			0x16					/*配置PLL芯片*/

#define CMD_TYPE_WRITE_SN				0X17/*设置SN*/
#define CMD_TYPE_READ_SN				0X18/*读取SN*/

#define CMD_TYPE_WATCGDOG_DBG			0x19

#define CMD_CH    (1)
#define CMD_CFG  (2)
#define CMD_CCH (3)

char *mcusoft_version = "V1.1.3.6-2016-5-9";          //MCU软件版本号

ccu_pc_buf_t ccu_pc_buf;
uart_buf_t ccu_uart_buf;
extern radio_param_t radio_param;
//static radio_param_t radio_param;

#define  PARA_LEN  20
#define  HEART_LEN 40
//#define  MAX_SIZE  20	//调试
#define  UART_BYE_FIFO_U 50
#define  band_w    1	//宽带,非1的为窄带

extern const char *comma;
extern const char *dot;
extern const char *enter;
extern const char *zero;
extern char RPM[10];
extern char g_mx920_info[256];
extern int32_t watchdog_cnt;
/****************************/
void resp_cmd_pots_query(void);
DIAG_buf_t resp_DIAG_buf;
extern int alarm_count;
/*****************************/
void init_software(void)
{
	init_ccu_buf();
	get_stored_param_from_flash();      
}

void init_ccu_uart(void)
{
	ccu_uart_buf.ring_head = 0;
	ccu_uart_buf.ring_tail = 0;
	ccu_uart_buf.stream_state = SS_SYN;
	//ccu_uart_buf.stream_state = SS_IDLE;
	ccu_uart_buf.com_rx_sequence = 0;
	ccu_uart_buf.com_received_flg = FALSE;
	ccu_uart_buf.com_send_ready_flg = TRUE;
	ccu_uart_buf.com_tx_len = 0;
	ccu_uart_buf.com_tx_sequence = 0;	
}

void __pll_reg0_set(uint32_t ch, uint32_t reg, uint32_t )


void proc_SMFREQ_cmd(char *buf, unsigned char len)
{
	
}

void proc_SSFREQ_cmd(char *buf, unsigned char len)
{
	
}
void proc_PLLSET_cmd(char *buf, unsigned char len)
{
	uint32_t param[10];
	uint32_t cnt=0;
	char *cmd = buf + strlen("AT+PLLSET=");

	memset(param,0,sizeof(param));

	while(1)
	{
		while((*cmd != ',') && (*cmd != CR) && (*cmd != LF))
		{
	        if((*cmd >'9')||(*cmd <'0'))
			{
				 UARTprintf("input error,%s\n\r",buf);
				 return;
			}
	        param[cnt] = 10*param[cnt]  + *cmd++ - '0'; 	
		}
		cnt++;
		if((*cmd == CR) || (*cmd == LF))
		{
			goto DONE;
		}

		cmd++;		
	}
DONE:
	if(param[0] > 1)
	{
		UARTprintf("input error,%s\n\r",buf);
		return;
	}
	if(param[1] > 5)
	{
		UARTprintf("input error,%s\n\r",buf);
		return;
	}

	switch(param[1])
	{
		case 0:
			break;
		case 1:
			break;
		case 2:
			break;
		case 3:
			break;
		case 4:
			break;
		case 5:
			break;
		default:
			UARTprintf("input error,%s\n\r",buf);
			return;

	}
	
	
}
void proc_PLLGET_cmd(char *buf, unsigned char len)
{
	
}


void proc_AT_cmd(char *buf, unsigned char len)
{
	uint32_t i=0;
	char cmd[32];
	
	memset(cmd,0,sizeof(cmd));
	while((buf[i] != '=') && (buf[i] != '\r') && (buf[i] != '\n'))
	{
		cmd[i]=buf[i];
		i++;
		if(i>sizeof(cmd))
		{
			UARTprintf("cmd too long,%s\n\r",buf);
			return;
		}
	}
	if(0==strcmp(cmd,"AT+SMFREQ"))
	{
		proc_SMFREQ_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"AT+SSFREQ"))
	{
		proc_SSFREQ_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"AT+PLLSET"))
	{
		proc_PLLSET_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"AT+PLLGET"))
	{
		proc_PLLGET_cmd(buf,len);
	}
	else
		UARTprintf("Input error,%s\n\r",buf);
	return;
}

void proc_uart_cmd(char *buf, unsigned char len)
{	
	proc_AT_cmd(buf,len);
	return;		
}


/*串口缓存函数(uart1)，接收CCU/PC发来的CH,CFG,CCH等命令*/
void proc_uart_buf(uart_info_t *uart)
{
	unsigned char c;
	
	if (uart->ring_tail == uart->ring_head)  /*串口缓存为空*/
		return;

	while(uart->ring_tail != uart->ring_head)
	{
		c = uart->ring_buf[uart->ring_head];
		uart->ring_head = (uart->ring_head + 1) % sizeof(uart->ring_buf);

		switch(uart->stream_state)
		{
			case STATE_IDLE:
				if ('A' == c)
				{
					uart->stream_state = STATE_HEAD;
				}
				break;
				
			case STATE_HEAD:
				if ('T' == c)
				{
					uart->stream_state = STATE_HEAD_DOT;
				}
				else
				{
					uart->stream_state = STATE_IDLE;
				}
				break;
			case STATE_HEAD_DOT:
				if ('+' == c)
				{
					uart->com_rx_sequence = 0;
					uart->stream_state = STATE_REV_START;
				}
				else
					uart->stream_state = STATE_IDLE;
				break;

			case STATE_REV_START:
				if (CR == c)
				{
					uart->stream_state = STATE_REV_END;
				}
				else
				{
					if(uart->com_rx_sequence >= MAX_COMM_PACKSIZE)
					{
						uart->stream_state = STATE_IDLE;
					}
					else
					{
						uart->com_rx_buf[uart->com_rx_sequence++] = c;
					}
				}
				break;

			case STATE_REV_END:
				if (LF == c)
				{
					uart->stream_state=STATE_IDLE;
					uart->com_rx_buf[uart->com_rx_sequence++] = c;
					proc_uart_cmd(flag_cmd, uart->com_rx_buf, uart->com_rx_sequence);
				}
				else
				{
					uart->stream_state=STATE_IDLE;
				}
				break;
			default:
				uart->stream_state = STATE_IDLE;
				break;
		}
	}
}

//CCU/PC UART接收中断服务程序
void UART_CCU_rx_isr(void)
{
	while (UARTCharsAvail(uart_for_ccu))
	{
		ccu_uart_buf.ring_buf[ccu_uart_buf.ring_tail] = (unsigned char)UARTCharGetNonBlocking(uart_for_ccu);	
		ccu_uart_buf.ring_tail = (ccu_uart_buf.ring_tail + 1) % MAX_COM_RING_PACKSIZE;
	}
}

//UART1中断服务程序
void UART1IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART1_BASE, true);                   //读取中断状态
    UARTIntClear(UART1_BASE, ulStatus);                              //清除中断状态     

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //接收中断或接收超时中断
		UART_CCU_rx_isr();
}


