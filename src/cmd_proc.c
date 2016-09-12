
#include "cmd_proc.h"



char *mcusoft_version = "V1.0.0.0-20160721";          //MCU软件版本号

ccu_pc_buf_t ccu_pc_buf;
uart_buf_t ccu_uart_buf;

/****************************/
extern radio_param_t radio_param;
extern int32_t watchdog_cnt;
extern pll_info_t pll_info[2];
extern double cp_map[16];
extern long g_adc_sample_buffer[64];
extern int32_t g_adc_sample_idx;
extern long g_adc_rssi[2];
extern volatile int32_t g_adc_working_flag;
/*****************************/


at_func_map_t at_func_map[]={
		{"AT_SMFREQ","AT_SMFREQ=X,X为PLL1频率,单位KHz",proc_SMFREQ_cmd},
		{"AT_SMPLLCP","AT_SMPLLCP=X,X为PLL泵电流值(0-15)",proc_SMPLLCP_cmd},
		{"AT_SSFREQ","AT_SSFREQ=X,X为PLL2频率,单位KHz",proc_SSFREQ_cmd},
		{"AT_RFCTRL","AT_RFCTRL=X,X为RF_CTRL脚控制电平,0-低,1-高",proc_RFCTRL_cmd},
		{"AT_INFO","AT_INFO,打印单片机信息",proc_INFO_cmd},
		{"AT_PLLINFO","AT_PLLINFO,打印PLL信息",proc_PLLINFO_cmd},
		{"AT_SCAN","AT_SCAN=f1,f2,fs,其中f1为扫描开始频率,f2为扫描结束频率,fs为扫描间隔,单位KHz",proc_SCAN_cmd},
		{NULL,NULL,NULL}
};



void init_software(void)
{
	init_ccu_buf();
	get_stored_param_from_flash();      
}


void __pll_reg0_set(uint32_t ch, uint32_t reg, uint32_t int_value, uint32_t frac)
{
	uint32_t value;

	value = ((int_value & 0x3FFF) << 15) + ((frac & 0xFFF) << 3) + (reg&0x7);

	myprintf("PLL:%d,REG:%d,VALUE=0x%08x\r\n",ch,reg,value);

	//pll_1_tx_data(value);
}

void __pll_reg1_set(uint32_t ch, uint32_t reg, uint32_t adj, uint32_t prescaler, uint32_t phase, uint32_t mod)
{
	uint32_t value;

	value = ((adj&1) << 28) + ((prescaler&1) << 27) + ((phase&0xFFF) << 15) + ((mod&0xFFF) << 3) + (reg&0x7);
	
	//pll_1_tx_data(value);

	myprintf("PLL:%d,REG:%d,VALUE=0x%08x\r\n",ch,reg,value);
}

void __pll_reg2_set(uint32_t ch, uint32_t reg, uint32_t noise_mode, uint32_t muxout, uint32_t reference_double, uint32_t reference_div_by_2,
						uint32_t r_count, uint32_t double_buffer, uint32_t charge_pump, uint32_t ldf, uint32_t ldp, uint32_t pd, uint32_t power_down, 
						uint32_t cp, uint32_t counter_reset)
{
	uint32_t value;

	value = ((noise_mode&0x3) << 29) + ((muxout&0x7) << 26) + ((reference_double&1) << 25) + ((reference_div_by_2&1) << 24) +
			((r_count&0x3FF) << 14) + ((double_buffer&1) << 13) + ((charge_pump&0xF) << 9) + ((ldf&1) << 8) +
			((ldp&1) << 7) + ((pd&1) << 6) + ((power_down&1) << 5) + ((cp&1) << 4) + ((counter_reset&1) << 3) + (reg&0x7);

	//pll_1_tx_data(value);

	myprintf("PLL:%d,REG:%d,VALUE=0x%08x\r\n",ch,reg,value);
}
void __pll_reg3_set(uint32_t ch, uint32_t reg, uint32_t band_select, uint32_t abp, uint32_t charge_cancelation, uint32_t csr, uint32_t clk_div_mode, 
						uint32_t clk_div_value)
{
	uint32_t value;
	
	value = ((band_select&1) << 23) + ((abp&1) << 22) + ((charge_cancelation&1) << 21) + ((csr&1) << 18) + 
			((clk_div_mode&3) << 15) + ((clk_div_value&0xFFF) << 3) + (reg&0x7);

	//pll_1_tx_data(value);

	myprintf("PLL:%d,REG:%d,VALUE=0x%08x\r\n",ch,reg,value);

}
void __pll_reg4_set(uint32_t ch, uint32_t reg, uint32_t feed_back_select, uint32_t rf_div_select, uint32_t band_select_clk_div, uint32_t vco_power_down,
						uint32_t mtld, uint32_t aux_output_select, uint32_t aux_out, uint32_t aux_output_power, uint32_t rf_out, uint32_t output_power)
{
	uint32_t value;

	value = ((feed_back_select&1) << 23) + ((rf_div_select&7) << 20) + ((band_select_clk_div&0xFF) << 12) + ((vco_power_down&1) << 11) + 
			((mtld&1) << 10) + ((aux_output_select&1) << 9) + ((aux_out&1) << 8) + ((aux_output_power&3) << 6) +
			((rf_out&1) << 5) + ((output_power&3) << 3) + (reg&0x7);

	//pll_1_tx_data(value);

	myprintf("PLL:%d,REG:%d,VALUE=0x%08x\r\n",ch,reg,value);

}

void __pll_reg5_set(uint32_t ch, uint32_t reg, uint32_t ld_pin_mode)
{
	uint32_t value;
	uint8_t tmp[4];

	value = ((ld_pin_mode&3) << 22) + (3<<19) + (reg&0x7);
	//pll_1_tx_data(value);
#if 0	
	tmp[3] = 0x00;
	tmp[2] = 0xd8;
	tmp[1] = 0x00;				//write communication register 0x00580005 to control the progress 
 	tmp[0] = 0x05;				//to write Register 5 to set digital lock detector
	WriteToADF4351_1(4,tmp);
#endif

	tmp[3] = 0x00;
	tmp[2] = 0x58;
	tmp[1] = 0x00;				//write communication register 0x00580005 to control the progress 
 	tmp[0] = 0x05;				//to write Register 5 to set digital lock detector
	WriteToADF4351_1(4,tmp);		

	tmp[3] = 0x00;
	tmp[2] = 0xCC;				//(DB23=1)The signal is taken from the VCO directly;(DB22-20:4H)the RF divider is 16;(DB19-12:50H)R is 80
	tmp[1] = 0x80;				//(DB11=0)VCO powerd up;
 	tmp[0] = 0x3C;				//(DB5=1)RF output is enabled;(DB4-3=3H)Output power level is 5
	WriteToADF4351_1(4,tmp);		

	tmp[3] = 0x00;
	tmp[2] = 0x00;
	tmp[1] = 0x04;				//(DB14-3:96H)clock divider value is 150.
 	tmp[0] = 0xB3;
	WriteToADF4351_1(4,tmp);

	tmp[3] = 0x00;
	tmp[2] = 0x00;				//(DB6=1)set PD polarity is positive;(DB7=1)LDP is 6nS;
	tmp[1] = 0x4E;				//(DB8=0)enable fractional-N digital lock detect;
 	tmp[0] = 0x42;				//(DB12-9:7H)set Icp 2.50 mA;
	WriteToADF4351_1(4,tmp);	//(DB23-14:1H)R counter is 1

	tmp[3] = 0x08;
	tmp[2] = 0x00;
	tmp[1] = 0x80;			   //(DB14-3:6H)MOD counter is 6;
 	tmp[0] = 0x11;			   //(DB26-15:6H)PHASE word is 1,neither the phase resync 
	WriteToADF4351_1(4,tmp);	   //nor the spurious optimization functions are being used
							   //(DB27=1)prescaler value is 8/9

	tmp[3] = 0x00;
	tmp[2] = 0x40;
	tmp[1] = 0x00;
 	tmp[0] = 0x00;				//(DB14-3:0H)FRAC value is 0;
	WriteToADF4351_1(4,tmp);		//(DB30-15:140H)INT value is 320;

	
	myprintf("PLL:%d,REG:%d,VALUE=0x%08x\r\n",ch,reg,value);

}




void proc_SMPLLCP_cmd(char *buf, unsigned char len)
{
	uint32_t param[64];
	uint32_t cnt=0;
	char *cmd = buf + strlen("AT_SMPLLCP=");

	memset(param,0,sizeof(param));

	while(1)
	{
		while((*cmd != ',') && (*cmd != CR) && (*cmd != LF))
		{
	        if((*cmd >'9')||(*cmd <'0'))
			{
				 myprintf("input error,%s\n\r",buf);
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
	if(cnt > 1)
	{
		myprintf("input error,%s\n\r",buf);
		return;
	}
	if(param[0]>=16)
	{
		myprintf("输入PLL泵电流错误,%d\n\r",param[0]);
		return;		
	}
	
	pll_info[0].cp_value = param[0];
	myprintf("主PLL泵电流为%fmA\n\r",cp_map[pll_info[0].cp_value]);
	
	rf_out(&pll_info[0]);
}



void proc_SMFREQ_cmd(char *buf, unsigned char len)
{
	uint32_t param[64];
	uint32_t cnt=0;
	char *cmd = buf + strlen("AT_SMFREQ=");

	memset(param,0,sizeof(param));

	while(1)
	{
		while((*cmd != ',') && (*cmd != CR) && (*cmd != LF))
		{
	        if((*cmd >'9')||(*cmd <'0'))
			{
				 myprintf("input error,%s\n\r",buf);
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
	if(cnt > 1)
	{
		myprintf("input error,%s\n\r",buf);
		return;
	}
	if((param[0]<(MIN_FREQ_HZ/FREQ_1_KHZ)) || (param[0]>(MAX_FREQ_HZ/FREQ_1_KHZ)))
	{
		myprintf("输入频率超出范围,%dHz\n\r",param[0]);
		return;		
	}
	param[0] = param[0]*FREQ_1_KHZ;
	pll_info[0].rf_freq_hz = param[0]-MID_FREQ_HZ;
	myprintf("主接收频率设置为%dHz,pll-1设置为:%dHz\n\r",param[0],pll_info[0].rf_freq_hz);
	
	rf_out(&pll_info[0]);
}

void proc_SSFREQ_cmd(char *buf, unsigned char len)
{
	
}
void proc_PLLSET_cmd(char *buf, unsigned char len)
{
	uint32_t param[64];
	uint32_t cnt=0;
	char *cmd = buf + strlen("AT_PLLSET=");

	memset(param,0,sizeof(param));

	while(1)
	{
		while((*cmd != ',') && (*cmd != CR) && (*cmd != LF))
		{
	        if((*cmd >'9')||(*cmd <'0'))
			{
				 myprintf("input error,%s\n\r",buf);
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
		myprintf("input error,%s\n\r",buf);
		return;
	}
	if(param[1] > 5)
	{
		myprintf("input error,%s\n\r",buf);
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
			myprintf("input error,%s\n\r",buf);
			return;

	}
	
	
}
void proc_PLLGET_cmd(char *buf, unsigned char len)
{
	
}


void proc_PLLINFO_cmd(char *buf, unsigned char len)
{
	printf_pll_info(&pll_info[0]);
}


void proc_RFCTRL_cmd(char *buf, unsigned char len)
{
	uint32_t param[64];
	uint32_t cnt=0;
	char *cmd = buf + strlen("AT_RFCTRL=");

	memset(param,0,sizeof(param));

	while(1)
	{
		while((*cmd != ',') && (*cmd != CR) && (*cmd != LF))
		{
			if((*cmd >'9')||(*cmd <'0'))
			{
				 myprintf("input error,%s\r\n",buf);
				 return;
			}
			param[cnt] = 10*param[cnt]	+ *cmd++ - '0'; 	
		}
		cnt++;
		if((*cmd == CR) || (*cmd == LF))
		{
			goto DONE;
		}

		cmd++;		
	}
DONE:
	if(cnt > 1)
	{
		myprintf("input error,%s\r\n",buf);
		return;
	}

	if(param[0] == 0)
	{
		CLR_RF_CTRL_1();
		myprintf("RF_CTRL置为低\r\n");
	}
	else
	{
		SET_RF_CTRL_1();
		myprintf("RF_CTRL置为高\r\n");
	}

}


void proc_SCAN_cmd(char *buf, unsigned char len)
{
	uint32_t param[64];
	uint32_t cnt=0;
	char *cmd = buf + strlen("AT_SCAN=");

	memset(param,0,sizeof(param));

	while(1)
	{
		while((*cmd != ',') && (*cmd != CR) && (*cmd != LF))
		{
	        if((*cmd >'9')||(*cmd <'0'))
			{
				 myprintf("input error,%s\r\n",buf);
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
	if(cnt > 3)
	{
		myprintf("input error,%s\r\n",buf);
		return;
	}
	if((param[0]<(MIN_FREQ_HZ/FREQ_1_KHZ)) || (param[0]>(MAX_FREQ_HZ/FREQ_1_KHZ)))
	{
		myprintf("输入扫描起始频率超出范围,%dKHz\r\n",param[0]);
		return;		
	}
	if((param[1]<(MIN_FREQ_HZ/FREQ_1_KHZ)) || (param[1]>(MAX_FREQ_HZ/FREQ_1_KHZ)))
	{
		myprintf("输入扫描结束频率超出范围,%dKHz\r\n",param[1]);
		return;		
	}
	if(param[0]>param[1])
	{
		myprintf("输入扫描频率错误,%d-%dKHz\r\n",param[0],param[1]);
		return;			
	}

	
	/*先使用PLL0扫描*/
	pll_info[0].scan_start_freq_hz = param[0]*FREQ_1_KHZ;
	pll_info[0].scan_stop_freq_hz = param[1]*FREQ_1_KHZ;
	pll_info[0].scan_span_hz = param[2]*FREQ_1_KHZ;
	
	myprintf("主pll-1扫描参数设置为:%dHz-%dHz,span:%dHz,将开始扫描\r\n",pll_info[0].scan_start_freq_hz,
				pll_info[0].scan_stop_freq_hz,pll_info[0].scan_span_hz);

	pll_info[0].scan_cur_freq_hz = 0;
	pll_info[0].scan_status=SCAN_STATUS_IDLE;
	pll_info[0].scan_enable=1;
	
}



void proc_INFO_cmd(char *buf, unsigned char len)
{
	myprintf("-----------单片机信息如下-------------\r\n");
	myprintf("软件版本:ver = %s\r\n",VER_INFO);
	myprintf("         date = %s %s\r\n", __DATE__,__TIME__);	
	myprintf("         watchdog_cnt = %d\r\n", watchdog_cnt);
}

void myhelp(void)
{

	uint32_t i=0;

	myprintf("输入错误!!!\n\r");
	while(at_func_map[i].func!=NULL)
	{
		myprintf("		%s\r\n",at_func_map[i++].info);
	}
	return;
	
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
			myprintf("cmd too long,%s\r\n",buf);
			return;
		}
	}
#if 1
	i=0;
	while(at_func_map[i].func!=NULL)
	{
		if(0==strcmp(cmd,at_func_map[i].cmd))
		{
			at_func_map[i].func(buf,len);
			return;
		}
		i++;
	}
	if(0==strcmp(cmd,"killme"))
	{
		*(uint16_t *)0x100=100;
		return;
	}

	myhelp();
	return;
#endif
#if 0	
	if(0==strcmp(cmd,"SMFREQ"))
	{
		proc_SMFREQ_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"SSFREQ"))
	{
		proc_SSFREQ_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"PLLSET"))
	{
		proc_PLLSET_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"PLLGET"))
	{
		proc_PLLGET_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"INFO"))
	{
		proc_INFO_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"PLLINFO"))
	{
		proc_PLLINFO_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"SMPLLCP"))
	{
		proc_SMPLLCP_cmd(buf,len);
	}
	else if(0==strcmp(cmd,"RFCTRL"))
	{
		proc_RFCTRL_cmd(buf,len);
	}	
	else
	{
		myprintf("Input error,%s\n\r",buf);
	}
	
	return;
#endif
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

		if (LF == c)
		{
			uart->com_rx_buf[uart->com_rx_sequence++] = c;
			proc_uart_cmd(uart->com_rx_buf, uart->com_rx_sequence);
			uart->com_rx_sequence=0;
		}
		else
		{
			uart->com_rx_buf[uart->com_rx_sequence++] = c;
			if(uart->com_rx_sequence>=MAX_COMM_PACKSIZE)
				uart->com_rx_sequence=0;
		}

#if 0		
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
				if ('_' == c)
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
#endif		
	}
}

void proc_adc_sample_result(void)
{
	int32_t i;

	g_adc_rssi[0]=0;
	for(i=0;i<g_adc_sample_idx;i++)
	{
		g_adc_rssi[0] += g_adc_sample_buffer[i];
	}
	g_adc_rssi[0] /= g_adc_sample_idx;
	
	return;
}

void init_adc_info(void)
{
	g_adc_rssi[0]=0;
	g_adc_rssi[1]=0;
	g_adc_sample_idx=0;
	g_adc_working_flag = ADC_STAUTS_IDLE;
}

void pll_scan_proc_in_idle_status(pll_info_t *pll_info)
{
	if(pll_info->scan_cur_freq_hz==0)
	{
		pll_info->scan_cur_freq_hz = pll_info->scan_start_freq_hz;
		printf_to_android("AT_SCAN_START\r\n");
	}
	else
	{
		pll_info->scan_cur_freq_hz += pll_info->scan_span_hz;
		if(pll_info->scan_cur_freq_hz>pll_info->scan_stop_freq_hz)
			pll_info->scan_cur_freq_hz=pll_info->scan_start_freq_hz;
	}
	pll_info->rf_freq_hz = pll_info->scan_cur_freq_hz-MID_FREQ_HZ;
	myprintf("主接收频率设置为%dHz,pll-1设置为:%dHz\n\r",pll_info->scan_cur_freq_hz,pll_info[0].rf_freq_hz);
	rf_out(pll_info);
	pll_info->scan_status = SCAN_STATUS_WAITTING_PLLLOCK;
	return;
}

void pll_scan_proc_in_waitting_plllock_status(pll_info_t *pll_info)
{
	if(g_adc_working_flag == ADC_STAUTS_IDLE)
	{
		g_adc_working_flag=ADC_STAUTS_SAMPLING;
		ADCProcessorTrigger(ADC_BASE, 0);
		pll_info->scan_status = SCAN_STATUS_READING_RSSI;
	}
	return;	
}

void pll_scan_proc_in_reading_rssi_status(pll_info_t *pll_info)
{
	if(g_adc_working_flag == ADC_STAUTS_SAMPLE_DONE)
	{
		proc_adc_sample_result();
		printf_to_android("AT_RSSI=%d,%d,%d\r\n",pll_info->num,pll_info->scan_cur_freq_hz/FREQ_1_KHZ,g_adc_rssi[pll_info->num-1]);
		myprintf("AT_RSSI=%d,%d,%d\r\n",pll_info->num,pll_info->scan_cur_freq_hz/FREQ_1_KHZ,g_adc_rssi[pll_info->num-1]);
		init_adc_info();

		if((pll_info->scan_cur_freq_hz+pll_info->scan_span_hz)>pll_info->scan_stop_freq_hz)
		{
			printf_to_android("AT_SCAN_STOP\r\n");
			pll_info->scan_enable=0;		
		}
		pll_info->scan_status = SCAN_STATUS_IDLE;
	}
	return;
}


void pll_scan_proc(pll_info_t *pll_info)
{
	if(pll_info->scan_enable==0)
		return;


	switch(pll_info->scan_status)
	{
		case SCAN_STATUS_IDLE:
			pll_scan_proc_in_idle_status(pll_info);
			break;
		case SCAN_STATUS_WAITTING_PLLLOCK:
			pll_scan_proc_in_waitting_plllock_status(pll_info);
			break;
		case SCAN_STATUS_READING_RSSI:
			pll_scan_proc_in_reading_rssi_status(pll_info);
			break;
		default:
			pll_info->scan_status=SCAN_STATUS_IDLE;
			pll_info->scan_enable=1;
			break;
	}
	
	
}



