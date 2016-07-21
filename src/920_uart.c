
#include "includes.h"

#define  PARA_LEN  20

#define	 INIT	0 
#define  PT   	1	
#define	 DG		2
#define  MX9XX   3   /*INFO? : ��ѯ*/

int PT_DG_flag = INIT;

//add chenp
radio_version_info_t g_radio_station; 
//end

//uart_info_t  g_uart_dbg;
const char *cmd_pt_head = "PT,";
const char *cmd_che_head = "CHE,002,";
const char *cmd_che_wideband = "0,D,S,N,C,C,W,N";      //���
const char *cmd_che_narrowband = "0,D,S,N,C,C,N,N";      //խ��
const char *cmd_che_930wideband = "W,N";      //���
const char *cmd_che_930narrowband = "N,N";      //խ��
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

/*���ݽṹ��ʼ��*/
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
/*���ݽṹresp_DIAG_buf��ʼ��*/
void init_resp_DIAG_buf(void)
{
      memset(&resp_DIAG_buf,0,sizeof(resp_DIAG_buf));
      resp_DIAG_buf.alarm_sequence=0;
      resp_DIAG_buf.forwpwr_sequence=0;
      resp_DIAG_buf.reflpwr_sequence=0;
      resp_DIAG_buf.tempera_sequence=0;

}
#if 0
/*ͨ�ô��ڷ��ͺ������������̨��������*/
void uart_send_datas(unsigned long uart_base,radio_uart_t *uart_data,char *data_2_send)
{
	unsigned short i, len;
	strcpy(uart_data->com_tx_buf,data_2_send);
	
    //UARTprintf("uart_data->com_tx_buf = %s\n\r",uart_data->com_tx_buf);//����
	
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

/*CHE����*/
void cmd_che(void)
{
	unsigned short i, len;
	unsigned long tx_int;                     //����Ƶ��ת��ΪMHz�����ڱ�����������
	unsigned long rx_int;                     //����Ƶ��ת��ΪMHz�����ڱ�����������
	char tx_freq_temp[PARA_LEN];     //�ַ�����ʽ�ķ���Ƶ��
	char rx_freq_temp[PARA_LEN];     //�ַ�����ʽ�Ľ���Ƶ��
	char tx_freq[PARA_LEN];               //��С����ķ���Ƶ��
	char rx_freq[PARA_LEN];               //��С����Ľ���Ƶ��
	char tx_pwr[PARA_LEN];                //�ַ�����ʽ�ķ��书��
    char soft_channel[PARA_LEN] = "CH002\r";             //�����ʽͨ��ѡ��
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
            strcpy( cmd_che_tail, cmd_che_930wideband );     //���
        }else if(MX920 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_wideband );     //���
        }
        
    }
    else
    {
        if(MX930 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_930narrowband );     //խ��
        }else if(MX920 == g_radio_station.mx9xx)
        {
            strcpy( cmd_che_tail, cmd_che_narrowband );     //խ��
        } 
    }

	strcpy(che_resp , cmd_che_head);              //����ͷ
	strcat(che_resp , tx_freq);                            //����Ƶ��
	
	if(MX920 == g_radio_station.mx9xx)   //mx920 ���͸�ʽ
	{
    	strcat(che_resp , comma);
    	strcat(che_resp , cmd_che_tx_subtone);    //txansmit subtone
    }
	
	strcat(che_resp , comma);
	strcat(che_resp , tx_pwr);                             //���书��
	strcat(che_resp , comma);
	strcat(che_resp , rx_freq);                            //����Ƶ��
	strcat(che_resp , comma);
    strcat(che_resp , cmd_che_tail);                        //��խ��
	strcat(che_resp , enter);                              //������"\r"

	UARTprintf("CHE����д������ = %s\n\r",che_resp);      //������
	
	uart_send_datas(uart_for_920,&g_uart_dbg,che_resp);	//��UART0����che_resp�����ݣ������̨������������
	
	while(g_uart_dbg.com_send_ready_flg == FALSE);
	
	//��ʱ1s
	//SysCtlDelay( (SysCtlClockGet() / 3)); 

    //��������"CH2"���������ʽͨ��ѡ��
    for(i=0; i< strlen(soft_channel); i++)
    {
        UARTCharPutNonBlocking(uart_for_920,soft_channel[i]);
    } 
    
    UARTprintf("�����ʽͨ��ѡ�� = %s\n\r",soft_channel);
}



/*PT����*/
void cmd_pt(void)
{
	unsigned short i, len;
	char tx_pow_ref[PARA_LEN];     //�ַ�����ʽ�ĵ�̨���书�ʲο���ƽ
	char rx_osc_ref[PARA_LEN];     //�ַ�����ʽ�ĵ�̨���侧�������ƽ
	char mod2_buf[PARA_LEN];      //�ַ�����ʽ�ĵ�̨ƽ��mod2
	char mod1_buf[PARA_LEN];      //�ַ�����ʽ�ĵ�̨ƽ��mod1
	char pt_resp[MAX_COMM_PACKSIZE];	//������̨���͵�PT,.....

	if((cur_param.pm.tx_power_setting == 0)
		|| (cur_param.pm.tx_ref_osci == 0)
		|| (cur_param.tx_parm.mod2 == 0)
		|| (cur_param.tx_parm.mod1 == 0))
	{
		UARTprintf("PT����д������Ϊ0������\n\r");   //������	
		return;
	}

	itoa(cur_param.pm.tx_power_setting, tx_pow_ref, 10);
	itoa(cur_param.pm.tx_ref_osci, rx_osc_ref, 10);
	itoa(cur_param.tx_parm.mod2, mod2_buf, 10);
	itoa(cur_param.tx_parm.mod1, mod1_buf, 10);
	
	strcpy(pt_resp , cmd_pt_head);    //����ͷ
	strcat(pt_resp , tx_pow_ref);          //��̨���书�ʲο���ƽ
	strcat(pt_resp , comma);
	strcat(pt_resp , rx_osc_ref);          //��̨���侧�������ƽ
	strcat(pt_resp , comma);
	strcat(pt_resp , mod2_buf);           //��̨ƽ��mod2
	strcat(pt_resp , comma);
	strcat(pt_resp , mod1_buf);           //��̨ƽ��mod1
	strcat(pt_resp , enter);                //������"\r"

	UARTprintf("PT����д������ = %s\n\r",pt_resp);   //������
	
	
	uart_send_datas(uart_for_920,&g_uart_dbg,pt_resp);	//��UART0����pt_resp�����ݣ������̨������������
	
	
}

/*POTS?�����ȡ��̨�б����PT�����������*/
/*����̨���书�ʲο���ƽ����̨���侧�������ƽ����̨ƽ��mod2����̨ƽ��mod1*/
void cmd_pots_query(void)
{
	unsigned short i, len;
	char pot_resp[MAX_COMM_PACKSIZE];
	
	strcpy(pot_resp, cmd_pots_que);
	strcat(pot_resp , enter);
	
	uart_send_datas(uart_for_920,&g_uart_dbg,pot_resp);	//��UART0����pot_resp�����ݣ������̨����
	
}

/*����POTS?���������������*/
static void handle_cmd_pots_query(const char *buf, unsigned char len)
{
	read_tx_power_setting=0;
	read_tx_ref_osci=0;
	//UARTprintf("POTS�����ѯ��� = PT,%s\n\r", buf);
	
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


/*handle_cmd_pots_query()ִ����Ϻ󣬷���PT����*/
void sent_pt(void)
{
	if (1 == flag_ready)
	{
		flag_ready = 0;
		cur_param.pm.tx_power_setting = read_tx_power_setting;
		cur_param.pm.tx_ref_osci = read_tx_ref_osci;
		//cmd_pt();//by zhongh 2012-12-13 17:17:14
		//UARTprintf("��̨���óɹ�\n\r");//by zhongh 2012-12-13 17:17:19
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
		UARTprintf("����δ֪��̨�ͺ�\n");
		return;
	}

	memset(g_mx920_info,0,sizeof(g_mx920_info));
	memcpy(g_mx920_info,buf,len);
	
	UARTprintf("�ŵ�����Ϣ:%s\n",g_mx920_info);

	g_radio_station.type_getted = 1;
	
	shutdown_radio_uart();
	
	return;
}


/*��ȡPOTS?�������Ӧ,��Ӧ��ʽ��PT,132,126,20,41;DIAG1������Ӧ��ʽ����DG,003,519.07500,519.02500,-----,-----,  0.0,  0.0,----,P0,20.8,MUTED, 9.0, 0.3,F0,13.6,00,10*/
void resp_cmd_pots_query(void)
{
	unsigned char c;
    char rx_flag = 0;
    //char buf[32] = {0};
    int iloop = 0;
	char tmp[128];
	
	if (g_uart_dbg.ring_tail == g_uart_dbg.ring_head)   //������ջ�����Ϊ��
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
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //��ʼ����"PT,"��"DG,"���������		
                    }else if((0 == strncmp(buff_cmd,"DG,",3)))
                    {
                        PT_DG_flag = DG; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //��ʼ����"PT,"��"DG,"���������		
                    }else if(0 == strncmp(buff_cmd,"MX930",5))
                    {
                        g_radio_station.mx9xx = MX930; //930
                        PT_DG_flag = MX9XX; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //��ʼ����"PT,"��"DG,"���������		
                    }else if(0 == strncmp(buff_cmd,"MX920",5))
                    {
                        g_radio_station.mx9xx = MX920; //920
                        PT_DG_flag = MX9XX; 
                        g_uart_dbg.com_rx_sequence = 0;
        				g_uart_dbg.stream_state = SS_BEGINRECV;  //��ʼ����"PT,"��"DG,"���������		
                    }
                }
                iloop = (iloop + 1) % sizeof(buff_cmd);
                break;
 #endif
 			case SS_SYN:
				if ('P' == c)
				{
					g_uart_dbg.stream_state = SS_IDLE;             		//�ж�"PT,"
				    PT_DG_flag = PT;                                       //��ǽ��յ�������PT,...
                }
                else if('D' == c)                                        //�ж�"DG"
                {
                    g_uart_dbg.stream_state = SS_IDLE;
                    PT_DG_flag = DG;                                       //��ǽ��յ�������DG,...
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
					g_uart_dbg.stream_state = SS_BEGINRECV;  //��ʼ����"PT,"��"DG,"���������
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
					g_uart_dbg.stream_state=SS_SYN;    				 //����"POTS?"��"DIAG1"����Ӧ�������                  
                    if(PT_DG_flag == PT)   								 //������ܵ�����Ӧ��PT,.....��ִ�д���PT�ĺ���
                     {	
						PT_DG_flag = INIT;								//��־λ���㣬�����´�ʹ��
						//handle_cmd_pots_query(g_uart_dbg.com_rx_buf, g_uart_dbg.com_rx_sequence);
                     }
				     else if(PT_DG_flag == DG)   						//������ܵ�����Ӧ��DG,.....��ִ�д���DG�ĺ���
                     {
                     	/*if(heart_flag == 0)				//���CCH��������ʱ��ȥ������Ӧ
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
/*���̨������������ DIAGM=FAST*/

  void cmd_DIAGM_que(void)
  {
	  unsigned short i, len;
  	  char diagm_resp[PARA_LEN];
	  strcpy(diagm_resp , cmd_DIAGM_FAST); //����DIAGM=FAST
	  strcat(diagm_resp , enter);

	  uart_send_datas(uart_for_920,&g_uart_dbg,diagm_resp); //��UART0����pot_resp�����ݣ������̨����
	   
  }

  /*���̨���� DIAG1 �����ȡ��̨ DG������Alarm status��PA forward power��PA reflected power��ֵ*/

void cmd_DIAG1_que(void)
{
	unsigned short i, len;
	char diag1_resp[PARA_LEN];
	
        strcpy(diag1_resp , cmd_DIAG1);    //����DIAG1
	strcat(diag1_resp , enter);

	
	uart_send_datas(uart_for_920,&g_uart_dbg,diag1_resp); //��UART0����pot_resp�����ݣ������̨����
	
}

/**���̨����DIAG0�ر�����**/
void cmd_DIAG0_que(void)
{
	unsigned short i, len;
	char diag0_resp[PARA_LEN];

    strcpy(diag0_resp , cmd_DIAG0);  //�ر�DIAG0
	strcat(diag0_resp , enter);
	
	
	uart_send_datas(uart_for_920,&g_uart_dbg,diag0_resp); //��UART0����pot_resp�����ݣ������̨����
	
}




/***************************************************************/
/*920��ȡAlarm status��PA forward power��PA reflected power;
buf������:
003, 519.07500,  519.02500,  -----,      -----,      0.0,      0.0,     ----,  P0,      20.8,  MUTED,    9.0,   0.3,   F0,      13.6,        00,           10;
id,  ����Ƶ��,   ����Ƶ��,   tx_subtone, rx_subtone, ǰ����, ������,VSWR,  ptt״̬, PA�¶�,����״̬, rx��ѹ,tx��ѹ,fan״̬, psu�����ѹ, Alarm status, led status*/

/*920��ȡAlarm status��PA forward power��PA reflected power;
buf������:
003, 519.07500, 519.02500, 0.0,      0.0,      ----,  P0,     I13M,     20.8,   -117.0, 9.0,    0.3,    F0,      13.6,        00,           10;
id,  ����Ƶ��,  ����Ƶ��,  ǰ����, ������, VSWR,  ptt״̬,tx����, PA�¶�, RSSI,   rx��ѹ, tx��ѹ, fan״̬, psu�����ѹ, Alarm status, led status*/
static void handle_cmd_DIAG1(const char *buf, unsigned char len)
{
    unsigned int count=0;        //�ۼƶ��Ÿ���
    resp_DIAG_buf.forwpwr_sequence = 0; 
	resp_DIAG_buf.reflpwr_sequence = 0;
	resp_DIAG_buf.alarm_sequence   = 0;
    resp_DIAG_buf.tempera_sequence= 0;

	//UARTprintf("DIAG1�����ѯ��� = DG,%s\n\r", buf);
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
                      
                      if(count == 5)	//�ۼƵ���������ţ��ö�������һ������֮�����PA forward power
                      {
                        buf++; 			 //�Թ��ö���
                        
                        while(*buf != ',')
                        resp_DIAG_buf.pa_forwpwr[resp_DIAG_buf.forwpwr_sequence++] = *buf++;
                        
                        ++count;
                      }
                      
                      if(count == 6)	//�ۼƵ����������ţ��ö�������һ������֮�����PA reflected power
                      {
                        buf++; 			 //�Թ��ö���
                        
                        while(*buf != ',')
                        resp_DIAG_buf.pa_reflpwr[resp_DIAG_buf.reflpwr_sequence++] = *buf++;
                        
                        ++count;
                      }

    		     if(count == 9)	//�ۼƵ��ھŸ����ţ��ö�������һ������֮�����PA temperature
                      {
                        buf++; 			 //�Թ��ö���
                        
                        while(*buf != ',')
                        resp_DIAG_buf.pa_temperature[resp_DIAG_buf.tempera_sequence++] = *buf++;
                        
                        ++count;
                      }

    			
                      if(count == 15)	//��ʮ������ź���Alarm status
                      {
                        buf++;  		//�Թ��ö���
                        
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
              
              if(count == 3)	//�ۼƵ���������ţ��ö�������һ������֮�����PA forward power
              {
                buf++; 			 //�Թ��ö���
                
                while(*buf != ',')
                resp_DIAG_buf.pa_forwpwr[resp_DIAG_buf.forwpwr_sequence++] = *buf++;
                
                ++count;
              }
              
              if(count == 4)	//�ۼƵ����������ţ��ö�������һ������֮�����PA reflected power
              {
                buf++; 			 //�Թ��ö���
                
                while(*buf != ',')
                resp_DIAG_buf.pa_reflpwr[resp_DIAG_buf.reflpwr_sequence++] = *buf++;
                
                ++count;
              }

	          if(count == 8)	//�ۼƵ��ھŸ����ţ��ö�������һ������֮�����PA temperature
              {
                buf++; 			 //�Թ��ö���
                
                while(*buf != ',')
                resp_DIAG_buf.pa_temperature[resp_DIAG_buf.tempera_sequence++] = *buf++;
                
                ++count;
              }

              if(count == 14)	//��ʮ������ź���Alarm status
              {
                buf++;  		//�Թ��ö���
                
                while(*buf != ',')
                resp_DIAG_buf.alarm_status[resp_DIAG_buf.alarm_sequence++] = *buf++;
                
                ++count;
				break;
              }
            }
          buf++; 	
    	}
    }

	
	/**���̨����DIAG0�ر�(״̬��ѯ)����**/
	//cmd_DIAG0_que();
	
    //UARTprintf("DIAG1������ȡ = resp_DIAG_buf.pa_forwpwr,%s\n\r",resp_DIAG_buf.pa_forwpwr);
	//UARTprintf("DIAG1������ȡ = resp_DIAG_buf.pa_reflpwr,%s\n\r",resp_DIAG_buf.pa_reflpwr);
	//UARTprintf("DIAG1������ȡ = resp_DIAG_buf.alarm_status,%s\n\r",resp_DIAG_buf.alarm_status);
	
}

/***************************************************************/




