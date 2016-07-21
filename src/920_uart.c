
#include "includes.h"

#define  PARA_LEN  20

#define	 INIT	0 
#define  PT   	1	
#define	 DG		2
#define  MX9XX   3   /*INFO? : 查询*/

int PT_DG_flag = INIT;

//add chenp
radio_version_info_t g_radio_station; 
//end

//uart_info_t  g_uart_dbg;
const char *cmd_pt_head = "PT,";
const char *cmd_che_head = "CHE,002,";
const char *cmd_che_wideband = "0,D,S,N,C,C,W,N";      //宽带
const char *cmd_che_narrowband = "0,D,S,N,C,C,N,N";      //窄带
const char *cmd_che_930wideband = "W,N";      //宽带
const char *cmd_che_930narrowband = "N,N";      //窄带
const char *cmd_che_tx_subtone = "0";
const char *cmd_pots_que = "POTS?";
const char *comma = ",";
const char *dot = ".";
const char *zero = "\0";
const char *enter = "\r";
/****************/
const char *cmd_DIAGM_FAST = "DIAGM=FAST";
const char *cmd_DIAG1="DIAG1";
const char *cmd_DIAG0="DIAG0";
/***************/
unsigned char flag_query_cops;

static unsigned char read_tx_power_setting;
static unsigned char read_tx_ref_osci;
static unsigned char flag_ready;

extern uart_info_t  g_uart_dbg;

/*数据结构初始化*/
void init_radio_uart(void)
{
	g_uart_dbg.ring_head = 0;
	g_uart_dbg.ring_tail = 0;
	g_uart_dbg.stream_state = SS_SYN;
	g_uart_dbg.com_rx_sequence = 0;
	g_uart_dbg.com_received_flg = FALSE;
	g_uart_dbg.com_send_ready_flg = TRUE;
	g_uart_dbg.com_tx_len = 0;
	g_uart_dbg.com_tx_sequence = 0;	

	flag_ready = 0;
	flag_query_cops = 0;
}
/*数据结构resp_DIAG_buf初始化*/
void init_resp_DIAG_buf(void)
{
      memset(&resp_DIAG_buf,0,sizeof(resp_DIAG_buf));
      resp_DIAG_buf.alarm_sequence=0;
      resp_DIAG_buf.forwpwr_sequence=0;
      resp_DIAG_buf.reflpwr_sequence=0;
      resp_DIAG_buf.tempera_sequence=0;

}
#if 0
/*通用串口发送函数，例如向电台发送命令*/
void uart_send_datas(unsigned long uart_base,radio_uart_t *uart_data,char *data_2_send)
{
	unsigned short i, len;
	strcpy(uart_data->com_tx_buf,data_2_send);
	
    //UARTprintf("uart_data->com_tx_buf = %s\n\r",uart_data->com_tx_buf);//调试
	
	while(uart_data->com_send_ready_flg == FALSE);
	
	uart_data->com_tx_len = len = strlen(data_2_send);
	if(len >= UART_BYE_FIFO)  len = UART_BYE_FIFO;
	for(i=0; i< len; i++)
	{
		UARTCharPutNonBlocking(uart_base, uart_data->com_tx_buf[i]);
	} 
    
	uart_data->com_tx_sequence = len;
	uart_data->com_send_ready_flg = FALSE;   	
}
#endif

/*CHE命令*/
void cmd_che(void)
{
	unsigned short i, len;
	unsigned long tx_int;                     //发射频率转换为MHz后，用于保存整数部分
	unsigned long rx_int;                     //接收频率转换为MHz后，用于保存整数部分
	char tx_freq_temp[PARA_LEN];     //字符串格式的发射频率
	char rx_freq_temp[PARA_LEN];     //字符串格式的接收频率
	char tx_freq[PARA_LEN];               //带小数点的发射频率
	char rx_freq[PARA_LEN];               //带小数点的接收频率
	char tx_pwr[PARA_LEN];                //字符串格式的发射功率
    char soft_channel[PARA_LEN] = "CH002\r";             //软件方式通道选择
    char cmd_che_tail[PARA_LEN];
	char che_resp[MAX_COMM_PACKSIZE];

	tx_int = cur_param.tx_freq / 1000000;
	rx_int = cur_param.rx_freq / 1000000;
	itoa(tx_int, tx_freq, 10);
	strcat(tx_freq, dot);
	itoa(cur_param.tx_freq, tx_freq_temp, 10);
	strcat(tx_freq, &tx_freq_temp[strlen(tx_freq)]-1);	
	itoa(rx_int, rx_freq, 10);
	strcat(rx_freq, dot);
	itoa(cur_param.rx_freq, rx_freq_temp, 10);
	strcat(rx_freq, &rx_freq_temp[strlen(rx_freq)]-1);	
	itoa(cur_param.power, tx_pwr, 10);
        
    if( cur_param.n_w_band == 1 )  
    {
        if(MX930 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_930wideband );     //宽带
        }else if(MX920 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_wideband );     //宽带
        }
        
    }
    else
    {
        if(MX930 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_930narrowband );     //窄带
        }else if(MX920 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_narrowband );     //窄带
        } 
    }

	strcpy(che_resp , cmd_che_head);              //命令头
	strcat(che_resp , tx_freq);                            //发射频率
	
	if(MX920 == g_radio_station.mx9xx)   //mx920 发送格式
	{
    	strcat(che_resp , comma);
    	strcat(che_resp , cmd_che_tx_subtone);    //txansmit subtone
    }
	
	strcat(che_resp , comma);
	strcat(che_resp , tx_pwr);                             //发射功率
	strcat(che_resp , comma);
	strcat(che_resp , rx_freq);                            //接收频率
	strcat(che_resp , comma);
    strcat(che_resp , cmd_che_tail);                        //宽窄带
	strcat(che_resp , enter);                              //结束符"\r"

	UARTprintf("CHE命令写入内容 = %s\n\r",che_resp);      //调试用
	
	uart_send_datas(uart_for_920,&g_uart_dbg,che_resp);	//向UART0发送che_resp的内容，即向电台发送配置命令
	
	while(g_uart_dbg.com_send_ready_flg == FALSE);
	
	//延时1s
	//SysCtlDelay( (SysCtlClockGet() / 3)); 

    //发送命令"CH2"，即软件方式通道选择
    for(i=0; i< strlen(soft_channel); i++)
    {
        UARTCharPutNonBlocking(uart_for_920,soft_channel[i]);
    } 
    
    UARTprintf("软件方式通道选择 = %s\n\r",soft_channel);
}



/*PT命令*/
void cmd_pt(void)
{
	unsigned short i, len;
	char tx_pow_ref[PARA_LEN];     //字符串格式的电台发射功率参考电平
	char rx_osc_ref[PARA_LEN];     //字符串格式的电台发射晶振参数电平
	char mod2_buf[PARA_LEN];      //字符串格式的电台平衡mod2
	char mod1_buf[PARA_LEN];      //字符串格式的电台平衡mod1
	char pt_resp[MAX_COMM_PACKSIZE];	//存放向电台发送的PT,.....

	if((cur_param.pm.tx_power_setting == 0)
		|| (cur_param.pm.tx_ref_osci == 0)
		|| (cur_param.tx_parm.mod2 == 0)
		|| (cur_param.tx_parm.mod1 == 0))
	{
		UARTprintf("PT命令写入内容为0，错误\n\r");   //调试用	
		return;
	}

	itoa(cur_param.pm.tx_power_setting, tx_pow_ref, 10);
	itoa(cur_param.pm.tx_ref_osci, rx_osc_ref, 10);
	itoa(cur_param.tx_parm.mod2, mod2_buf, 10);
	itoa(cur_param.tx_parm.mod1, mod1_buf, 10);
	
	strcpy(pt_resp , cmd_pt_head);    //命令头
	strcat(pt_resp , tx_pow_ref);          //电台发射功率参考电平
	strcat(pt_resp , comma);
	strcat(pt_resp , rx_osc_ref);          //电台发射晶振参数电平
	strcat(pt_resp , comma);
	strcat(pt_resp , mod2_buf);           //电台平衡mod2
	strcat(pt_resp , comma);
	strcat(pt_resp , mod1_buf);           //电台平衡mod1
	strcat(pt_resp , enter);                //结束符"\r"

	UARTprintf("PT命令写入内容 = %s\n\r",pt_resp);   //调试用
	
	
	uart_send_datas(uart_for_920,&g_uart_dbg,pt_resp);	//向UART0发送pt_resp的内容，即向电台发送配置命令
	
	
}

/*POTS?命令，读取电台中保存的PT命令四项参数*/
/*即电台发射功率参考电平，电台发射晶振参数电平，电台平衡mod2，电台平衡mod1*/
void cmd_pots_query(void)
{
	unsigned short i, len;
	char pot_resp[MAX_COMM_PACKSIZE];
	
	strcpy(pot_resp, cmd_pots_que);
	strcat(pot_resp , enter);
	
	uart_send_datas(uart_for_920,&g_uart_dbg,pot_resp);	//向UART0发送pot_resp的内容，即向电台发送
	
}

/*处理POTS?命令读出来的数据*/
static void handle_cmd_pots_query(const char *buf, unsigned char len)
{
	read_tx_power_setting=0;
	read_tx_ref_osci=0;
	//UARTprintf("POTS命令查询结果 = PT,%s\n\r", buf);
	
	while(*buf != ',')
	{
	        if(((*buf >'9')||(*buf <'0'))&&(*buf != ' '))
			{
			  UARTprintf("Wrong PT 1!\n\r");
			  return;
	        }
        	read_tx_power_setting = read_tx_power_setting * 10 + (*buf++ - '0');
	}
	buf++;
	while(*buf != ',')
	{
	       if(((*buf >'9')||(*buf <'0'))&&(*buf != ' '))
		   {
			  UARTprintf("Wrong PT 2!\n\r");
			  return;
	       }
           read_tx_ref_osci = read_tx_ref_osci * 10 + (*buf++ - '0');
	}

	if (1 == flag_query_cops)
	{
		flag_query_cops = 0;
		return;
	}

	flag_ready = 1;
	
}


/*handle_cmd_pots_query()执行完毕后，发送PT命令*/
void sent_pt(void)
{
	if (1 == flag_ready)
	{
		flag_ready = 0;
		cur_param.pm.tx_power_setting = read_tx_power_setting;
		cur_param.pm.tx_ref_osci = read_tx_ref_osci;
		//cmd_pt();//by zhongh 2012-12-13 17:17:14
		//UARTprintf("电台配置成功\n\r");//by zhongh 2012-12-13 17:17:19
		return;//by zhongh 2012-12-13 17:17:7
	}
}

/*
INFO?       Query model, serial number and firmware information
                MX920L3L3H,040610021,0.2.1,1,G2,L
                     |         |       |   |  | |
                     |         |       |   |  | +-- BOM revision (MX920 Common Components)
                     |         |       |   |  +---- PCB revision
                     |         |       |   +------- Model revision
                     |         |       +----------- Firmware version
                     |         +------------------- MX920 serial number
                     +----------------------------- MX920 model number

*/
char g_mx920_info[256];
static void handle_cmd_INFO(const char *buf, unsigned char len)
{
	char tmp[4];

	memcpy(tmp,buf,3);
	tmp[3]=0;

    if(0 == strcmp(tmp,"920"))
    {
        g_radio_station.mx9xx = MX920;
    }else if(0 == strcmp(tmp,"930"))
    {
        g_radio_station.mx9xx = MX930;
    }
	else
	{
		UARTprintf("读到未知电台型号\n");
		return;
	}

	memset(g_mx920_info,0,sizeof(g_mx920_info));
	memcpy(g_mx920_info,buf,len);
	
	UARTprintf("信道机信息:%s\n",g_mx920_info);

	g_radio_station.type_getted = 1;
	
	shutdown_radio_uart();
	
	return;
}


/*读取POTS?命令的响应,响应格式例PT,132,126,20,41;DIAG1命令响应格式例如DG,003,519.07500,519.02500,-----,-----,  0.0,  0.0,----,P0,20.8,MUTED, 9.0, 0.3,F0,13.6,00,10*/
void resp_cmd_pots_query(void)
{
	unsigned char c;
    char rx_flag = 0;
    //char buf[32] = {0};
    int iloop = 0;
	char tmp[128];
	
	if (g_uart_dbg.ring_tail == g_uart_dbg.ring_head)   //如果接收缓冲区为空
		return;

	memset(tmp,0,sizeof(tmp));

	while(g_uart_dbg.ring_tail != g_uart_dbg.ring_head)
	{
		c = g_uart_dbg.ring_buf[g_uart_dbg.ring_head];
		g_uart_dbg.ring_head = (g_uart_dbg.ring_head + 1) % MAX_COM_RING_PACKSIZE;

		tmp[iloop++] = c;

		switch(g_uart_dbg.stream_state)
		{
#if 0
			case SS_SYN:
                if ((buff_cmd[iloop] = c) == ',')
                {
                    if((0 == strncmp(buff_cmd,"PT,",3)))
                    {
                        PT_DG_flag = PT; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //开始接收"PT,"或"DG,"后面的内容		
                    }else if((0 == strncmp(buff_cmd,"DG,",3)))
                    {
                        PT_DG_flag = DG; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //开始接收"PT,"或"DG,"后面的内容		
                    }else if(0 == strncmp(buff_cmd,"MX930",5))
                    {
                        g_radio_station.mx9xx = MX930; //930
                        PT_DG_flag = MX9XX; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //开始接收"PT,"或"DG,"后面的内容		
                    }else if(0 == strncmp(buff_cmd,"MX920",5))
                    {
                        g_radio_station.mx9xx = MX920; //920
                        PT_DG_flag = MX9XX; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //开始接收"PT,"或"DG,"后面的内容		
                    }
                }
                iloop = (iloop + 1) % sizeof(buff_cmd);
                break;
 #endif
 			case SS_SYN:
				if ('P' == c)
				{
					g_uart_dbg.stream_state = SS_IDLE;             		//判断"PT,"
				    PT_DG_flag = PT;                                       //标记接收的数据是PT,...
                }
                else if('D' == c)                                        //判断"DG"
                {
                    g_uart_dbg.stream_state = SS_IDLE;
                    PT_DG_flag = DG;                                       //标记接收的数据是DG,...
                }else if('M' == c)  
                {
                	if(g_radio_station.info_cmd_sended)
                	{
                		g_uart_dbg.stream_state = SS_IDLE;
                    	PT_DG_flag = MX9XX;
                	}            
                }
				break;
				
			case SS_IDLE:
				if ('T' == c)
				{
					g_uart_dbg.stream_state = SS_T_IDLE;
				}
                else if('G' == c)
                {
                    g_uart_dbg.stream_state = SS_T_IDLE;
                }else if('X' == c) 
                {
					if(PT_DG_flag == MX9XX)
					{
						g_uart_dbg.com_rx_sequence = 0;
						g_uart_dbg.stream_state = SS_BEGINRECV;
					}
					else
						g_uart_dbg.stream_state = SS_T_IDLE;
                }
				break;

			case SS_T_IDLE:
				if (',' == c)
				{
					g_uart_dbg.com_rx_sequence = 0;
					g_uart_dbg.stream_state = SS_BEGINRECV;  //开始接收"PT,"或"DG,"后面的内容
				}
#if 0
				else 
				{
                    buf[iloop++] = c;
                    if(iloop > 3)
                    {
                        iloop = 0;
                        if(0 == strncmp(buf,"920",3))
                        {
                            g_radio_station.mx9xx = MX920;
                        }else if(0 == strncmp(buf,"930",3))
                        {
                            g_radio_station.mx9xx = MX930;
                        }
                    }
                }
#endif
				break;
			case SS_BEGINRECV:
				if (CR == c)
				{
					g_uart_dbg.stream_state = SS_CR_RECV;
				}
				else
				{
					if(g_uart_dbg.com_rx_sequence >= MAX_COMM_PACKSIZE)
					{
						g_uart_dbg.stream_state = SS_SYN;
					}
					else
					{
						g_uart_dbg.com_rx_buf[g_uart_dbg.com_rx_sequence++] = c;
					}
				}
				break;

			case SS_CR_RECV:
				if (CR == c)
				{
					g_uart_dbg.stream_state = SS_BEGINRECV;
					if(g_uart_dbg.com_rx_sequence >= MAX_COMM_PACKSIZE)
					{
						g_uart_dbg.stream_state = SS_SYN;
					}
					else
					{
						g_uart_dbg.com_rx_buf[g_uart_dbg.com_rx_sequence++] = c;
					}
				}
				else if (LF == c)
				{
					g_uart_dbg.stream_state=SS_SYN;    				 //整条"POTS?"或"DIAG1"的响应接收完毕                  
                    if(PT_DG_flag == PT)   								 //如果接受到的响应是PT,.....则执行处理PT的函数
                     {	
						PT_DG_flag = INIT;								//标志位清零，便于下次使用
						//handle_cmd_pots_query(g_uart_dbg.com_rx_buf, g_uart_dbg.com_rx_sequence);
                     }
				     else if(PT_DG_flag == DG)   						//如果接受到的响应是DG,.....则执行处理DG的函数
                     {
                     	/*if(heart_flag == 0)				//如果CCH心跳请求超时则不去处理响应
                  			  	{
							return;
						}*/
						
						PT_DG_flag= INIT;
                        handle_cmd_DIAG1(g_uart_dbg.com_rx_buf, g_uart_dbg.com_rx_sequence);
                     }
					 else if(PT_DG_flag == MX9XX)
					 {
					 	PT_DG_flag= INIT;
                        handle_cmd_INFO(g_uart_dbg.com_rx_buf, g_uart_dbg.com_rx_sequence);
					 }
                }
				break;
			default:
				g_uart_dbg.stream_state = SS_SYN;
				break;
		}
	}

	//UARTprintf("920 out:%s\n",tmp);
}

/**************************************************************/
/*向电台发送配置命令 DIAGM=FAST*/

  void cmd_DIAGM_que(void)
  {
	  unsigned short i, len;
  	  char diagm_resp[PARA_LEN];
	  strcpy(diagm_resp , cmd_DIAGM_FAST); //配置DIAGM=FAST
	  strcat(diagm_resp , enter);

	  uart_send_datas(uart_for_920,&g_uart_dbg,diagm_resp); //向UART0发送pot_resp的内容，即向电台发送
	   
  }

  /*向电台发送 DIAG1 命令，获取电台 DG，…，Alarm status，PA forward power，PA reflected power等值*/

void cmd_DIAG1_que(void)
{
	unsigned short i, len;
	char diag1_resp[PARA_LEN];
	
        strcpy(diag1_resp , cmd_DIAG1);    //开启DIAG1
	strcat(diag1_resp , enter);

	
	uart_send_datas(uart_for_920,&g_uart_dbg,diag1_resp); //向UART0发送pot_resp的内容，即向电台发送
	
}

/**向电台发送DIAG0关闭命令**/
void cmd_DIAG0_que(void)
{
	unsigned short i, len;
	char diag0_resp[PARA_LEN];

    strcpy(diag0_resp , cmd_DIAG0);  //关闭DIAG0
	strcat(diag0_resp , enter);
	
	
	uart_send_datas(uart_for_920,&g_uart_dbg,diag0_resp); //向UART0发送pot_resp的内容，即向电台发送
	
}




/***************************************************************/
/*920提取Alarm status，PA forward power，PA reflected power;
buf中内容:
003, 519.07500,  519.02500,  -----,      -----,      0.0,      0.0,     ----,  P0,      20.8,  MUTED,    9.0,   0.3,   F0,      13.6,        00,           10;
id,  发送频率,   接收频率,   tx_subtone, rx_subtone, 前向功率, 反向功率,VSWR,  ptt状态, PA温度,亚音状态, rx电压,tx电压,fan状态, psu输出电压, Alarm status, led status*/

/*920提取Alarm status，PA forward power，PA reflected power;
buf中内容:
003, 519.07500, 519.02500, 0.0,      0.0,      ----,  P0,     I13M,     20.8,   -117.0, 9.0,    0.3,    F0,      13.6,        00,           10;
id,  发送频率,  接收频率,  前向功率, 反向功率, VSWR,  ptt状态,tx振荡器, PA温度, RSSI,   rx电压, tx电压, fan状态, psu输出电压, Alarm status, led status*/
static void handle_cmd_DIAG1(const char *buf, unsigned char len)
{
    unsigned int count=0;        //累计逗号个数
    resp_DIAG_buf.forwpwr_sequence = 0; 
	resp_DIAG_buf.reflpwr_sequence = 0;
	resp_DIAG_buf.alarm_sequence   = 0;
    resp_DIAG_buf.tempera_sequence= 0;

	//UARTprintf("DIAG1命令查询结果 = DG,%s\n\r", buf);
	memset(resp_DIAG_buf.pa_forwpwr,0,sizeof(resp_DIAG_buf.pa_forwpwr));
	memset(resp_DIAG_buf.pa_reflpwr,0,sizeof(resp_DIAG_buf.pa_reflpwr));
	memset(resp_DIAG_buf.alarm_status,0,sizeof(resp_DIAG_buf.alarm_status));
	memset(resp_DIAG_buf.pa_temperature,0,sizeof(resp_DIAG_buf.pa_temperature));

	if((buf == NULL) || (len == 0))
	{
		return;
	}

    if(MX920 == g_radio_station.mx9xx)
    {
    	while(len-- )
    	{
    	       /* if(((*buf >'9')||(*buf <'0'))&&(*buf != ' '))
    			{
    			  UARTprintf("Wrong DG!\n\r");
    			  return;
    	    	}*/
                    
                    if(*buf == ',')
                    {
                      ++count;
                      
                      if(count == 5)	//累计到第五个逗号，该逗号与下一个逗号之间的是PA forward power
                      {
                        buf++; 			 //略过该逗号
                        
                        while(*buf != ',')
                        resp_DIAG_buf.pa_forwpwr[resp_DIAG_buf.forwpwr_sequence++] = *buf++;
                        
                        ++count;
                      }
                      
                      if(count == 6)	//累计到第六个逗号，该逗号与下一个逗号之间的是PA reflected power
                      {
                        buf++; 			 //略过该逗号
                        
                        while(*buf != ',')
                        resp_DIAG_buf.pa_reflpwr[resp_DIAG_buf.reflpwr_sequence++] = *buf++;
                        
                        ++count;
                      }

    		     if(count == 9)	//累计到第九个逗号，该逗号与下一个逗号之间的是PA temperature
                      {
                        buf++; 			 //略过该逗号
                        
                        while(*buf != ',')
                        resp_DIAG_buf.pa_temperature[resp_DIAG_buf.tempera_sequence++] = *buf++;
                        
                        ++count;
                      }

    			
                      if(count == 15)	//第十五个逗号后是Alarm status
                      {
                        buf++;  		//略过该逗号
                        
                        while(*buf != ',')
                        resp_DIAG_buf.alarm_status[resp_DIAG_buf.alarm_sequence++] = *buf++;
                        
                        ++count;
    					break;
                      }
                    }
              buf++; 	
    	}
    }else if(MX930 == g_radio_station.mx9xx)
    {
    	while(len-- )
    	{
	       /* if(((*buf >'9')||(*buf <'0'))&&(*buf != ' '))
			{
			  UARTprintf("Wrong DG!\n\r");
			  return;
	    	}*/
            
            if(*buf == ',')
            {
              ++count;
              
              if(count == 3)	//累计到第五个逗号，该逗号与下一个逗号之间的是PA forward power
              {
                buf++; 			 //略过该逗号
                
                while(*buf != ',')
                resp_DIAG_buf.pa_forwpwr[resp_DIAG_buf.forwpwr_sequence++] = *buf++;
                
                ++count;
              }
              
              if(count == 4)	//累计到第六个逗号，该逗号与下一个逗号之间的是PA reflected power
              {
                buf++; 			 //略过该逗号
                
                while(*buf != ',')
                resp_DIAG_buf.pa_reflpwr[resp_DIAG_buf.reflpwr_sequence++] = *buf++;
                
                ++count;
              }

	          if(count == 8)	//累计到第九个逗号，该逗号与下一个逗号之间的是PA temperature
              {
                buf++; 			 //略过该逗号
                
                while(*buf != ',')
                resp_DIAG_buf.pa_temperature[resp_DIAG_buf.tempera_sequence++] = *buf++;
                
                ++count;
              }

              if(count == 14)	//第十五个逗号后是Alarm status
              {
                buf++;  		//略过该逗号
                
                while(*buf != ',')
                resp_DIAG_buf.alarm_status[resp_DIAG_buf.alarm_sequence++] = *buf++;
                
                ++count;
				break;
              }
            }
          buf++; 	
    	}
    }

	
	/**向电台发送DIAG0关闭(状态查询)命令**/
	//cmd_DIAG0_que();
	
    //UARTprintf("DIAG1命令提取 = resp_DIAG_buf.pa_forwpwr,%s\n\r",resp_DIAG_buf.pa_forwpwr);
	//UARTprintf("DIAG1命令提取 = resp_DIAG_buf.pa_reflpwr,%s\n\r",resp_DIAG_buf.pa_reflpwr);
	//UARTprintf("DIAG1命令提取 = resp_DIAG_buf.alarm_status,%s\n\r",resp_DIAG_buf.alarm_status);
	
}

/***************************************************************/




