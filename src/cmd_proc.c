
#include "includes.h"



char *mcusoft_version = "V1.0.0.0-20160721";          //MCU软件版本号

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

void __pll_reg0_set(uint32_t ch, uint32_t reg, uint32_t int_value, uint32_t frac)
{
	uint32_t value;

	value = ((int_value & 0x3FFF) << 15) + ((frac & 0xFFF) << 3) + (reg&0x7);
}

void __pll_reg1_set(uint32_t ch, uint32_t reg, uint32_t adj, uint32_t prescaler, uint32_t phase, uint32_t mod)
{
	uint32_t value;

	value = ((adj&1) << 28) + ((prescaler&1) << 27) + ((phase&0xFFF) << 15) + ((mod&0xFFF) << 3) + (reg&0x7);
}

void __pll_reg2_set(uint32_t ch, uint32_t reg, uint32_t noise_mode, uint32_t muxout, uint32_t reference_double, uint32_t reference_div_by_2,
						uint32_t r_count, uint32_t double_buffer, uint32_t charge_pump, uint32_t ldf, uint32_t ldp, uint32_t pd, uint32_t power_down, 
						uint32_t cp, uint32_t counter_reset)
{
	uint32_t value;

	value = ((noise_mode&0x3) << 29) + ((muxout&0x7) << 26) + ((reference_double&1) << 25) + ((reference_div_by_2&1) << 24) +
			((r_count&0x3FF) << 14) + ((double_buffer&1) << 13) + ((charge_pump&0xF) << 9) + ((ldf&1) << 8) +
			((ldp&1) << 7) + ((pd&1) << 6) + ((power_down&1) << 5) + ((cp&1) << 4) + ((counter_reset&1) << 3) + (reg&0x7);
}
void __pll_reg3_set(uint32_t ch, uint32_t reg, uint32_t band_select, uint32_t abp, uint32_t charge_cancelation, uint32_t csr, uint32_t clk_div_mode, 
						uint32_t clk_div_value)
{
	uint32_t value;
	
	value = ((band_select&1) << 23) + ((abp&1) << 22) + ((charge_cancelation&1) << 21) + ((csr&1) << 18) + 
			((clk_div_mode&3) << 15) + ((clk_div_value&0xFFF) << 3) + (reg&0x7);

}
void __pll_reg4_set(uint32_t ch, uint32_t reg, uint32_t feed_back_select, uint32_t rf_div_select, uint32_t band_select_clk_div, uint32_t vco_power_down,
						uint32_t mtld, uint32_t aux_output_select, uint32_t aux_out, uint32_t aux_output_power, uint32_t rf_out, uint32_t output_power)
{
	uint32_t value;

	value = ((feed_back_select&1) << 23) + ((rf_div_select&7) << 20) + ((band_select_clk_div&0xFF) << 12) + ((vco_power_down&1) << 11) + 
			((mtld&1) << 10) + ((aux_output_select&1) << 9) + ((aux_out&1) << 8) + ((aux_output_power&3) << 6) +
			((rf_out&1) << 5) + ((output_power&3) << 3) + (reg&0x7);

}

void __pll_reg5_set(uint32_t ch, uint32_t reg, uint32_t ld_pin_mode)
{
	uint32_t value;

	value = ((ld_pin_mode&3) << 22) + (reg&0x7);

}





void proc_SMFREQ_cmd(char *buf, unsigned char len)
{
	
}

void proc_SSFREQ_cmd(char *buf, unsigned char len)
{
	
}
void proc_PLLSET_cmd(char *buf, unsigned char len)
{
	uint32_t param[64];
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
			__pll_reg0_set(param[0],param[1],param[2],param[3]);
			break;
		case 1:
			__pll_reg1_set(param[0],param[1],param[2],param[3],param[4],param[5]);
			break;
		case 2:
			__pll_reg2_set(param[0],param[1],param[2],param[3],param[4],param[5],param[6],param[7],param[8],param[9],param[10],param[11],
							param[12],param[13],param[14]);
			break;
		case 3:
			__pll_reg3_set(param[0],param[1],param[2],param[3],param[4],param[5],param[6],param[7]);
			break;
		case 4:
			__pll_reg4_set(param[0],param[1],param[2],param[3],param[4],param[5],param[6],param[7],param[8],param[9],param[10],param[11]);
			break;
		case 5:
			__pll_reg5_set(param[0],param[1],param[2]);
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
					proc_uart_cmd(uart->com_rx_buf, uart->com_rx_sequence);
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




