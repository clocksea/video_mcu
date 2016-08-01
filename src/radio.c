/********************************************************************
��Ȩ����	��Copyright (C) 2011 ����ͨ��
�ļ�����	�����õ�̨��������
�ļ�����	�����õ�̨����
��������	��2011-06-09
������		��ZhongH
�޸ı�ʶ	��
�޸�����	��
*********************************************************************/
#include "includes.h"



extern char g_mx920_info[256];
extern char *mcusoft_version;
cur_param_t  cur_param;
radio_param_t radio_param;

uint32_t radio_strlen(char *s)
{
	uint32_t i;
	for(i=0;s[i];i++);
	return i;
}

void init_ccu_buf(void)
{
	ccu_pc_buf.flag=0;
	FlashUsecSet(SysCtlClockGet() / 1000000);   //����ÿus��CPUʱ����
	memset(&ccu_pc_buf.buf,0,sizeof(ccu_pc_buf.buf));
	memset(&ccu_uart_buf,0,sizeof(ccu_uart_buf));
	memset(&radio_param,0,sizeof(radio_param));
	memset(&cur_param,0,sizeof(cur_param));
}

/*iicд����*/
void iic_write(uint8_t addr,uint8_t data)
{
  	  // ��Ҫ���͵��ַ����õ����ݼĴ����� 
	  I2CMasterDataPut(I2C0_MASTER_BASE,addr);   // �������ַ����������͵��ӻ��� 
	  I2CMasterControl(I2C0_MASTER_BASE,I2C_MASTER_CMD_BURST_SEND_START); // ��ʱһ��ʱ�䣬ֱ��������ɡ�
	  while(I2CMasterBusy(I2C0_MASTER_BASE));    // ��Ҫ���͵��ַ����õ����ݼĴ����С� 
	  I2CMasterDataPut(I2C0_MASTER_BASE,data);   // �������ַ����������͵��ӻ���
	  I2CMasterControl(I2C0_MASTER_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);  // ��ʱһ��ʱ�䣬ֱ��������ɡ� 
	  while(I2CMasterBusy(I2C0_MASTER_BASE)); 
}

/*��flash�ж�ȡ����*/
void get_stored_param_from_flash(void)
{
	memcpy(&radio_param,(void *)FLASH_BASE_ADDR,sizeof(radio_param));
}

/**/
uint16_t get_param_from_current_tx_freq(uint32_t tx_freq, ccu_stored_tx_parm_t *tx_param)
{
	tx_param ->freq = cur_param.tx_freq;

	if(tx_param ->freq == 0)
       {
                return 0; 					/*��ʾû�в���*/
       }
	else		
	{
		tx_freq = tx_param->freq ;
		return 1;
	}	
}

/*��radio_paramд��flash*/
/*pdata: ָ�����ݻ�������ָ��*/
/*addr: (addr+FLASH_BASE_ADDR)Ϊ��̵���ʼ��ַ��������4�ı���*/
/*size: ��̵��ֽ�����������4�ı���*/
static void radio_param_write_to_flash(unsigned long *pdata, unsigned long addr, unsigned long size)
{
	if (FlashErase(FLASH_BASE_ADDR) != 0)  //����1K��С��Flash��Flash��ʼ��ַΪFLASH_BASE_ADDR
	{
		UARTprintf("Flash������������δ��д��\n\r");      
		return;
	}
	//if (FlashProgram((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param))) 
	if (FlashProgram(pdata, addr, size)) 
	{
		UARTprintf("Flash��̳�������δ��д��\n\r");      
		return;
	}
}


/*��ð�ݷ���tx_pram[]������������*/
/*buf: ָ����Ҫ��������ݣ�n: ��Ҫ��������ݸ���*/
void bubble_tx_pram(ccu_stored_tx_parm_t *buf, uint32_t n)
{
	uint32_t i;
	uint32_t j;
	ccu_stored_tx_parm_t  tx_parm_temp;

	for (i=0; i<n-1; i++)
	{
		for (j=0; j<n-i-1; j++)
		{
			if (buf[j+1].freq < buf[j].freq)
			{
				memcpy(&tx_parm_temp, &radio_param.tx_parm[j], sizeof(tx_parm_temp));
				memcpy(&radio_param.tx_parm[j], &radio_param.tx_parm[j+1], sizeof(tx_parm_temp));
				memcpy(&radio_param.tx_parm[j+1], &tx_parm_temp, sizeof(tx_parm_temp));
			}
		}
	}
}

/*��MCU����汾*/
void read_mcu_version(void)
{
	UARTprintf("MCU����汾�� = %s\n\r", mcusoft_version);
}

/*�����õ�̨ƽ�����*/
void proc_CFG_pt(void)
{
	cmd_pt();
}


/*�����õ�̨����Ƶ�ʣ�����Ƶ�ʺͷ��书��*/
void proc_CFG_tx_rx_power(char *buf)              //10
{
	unsigned long tx_frequency = 0;
       unsigned long rx_frequency = 0;
	unsigned char tx_power;
      
	while(*buf != ',')
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               tx_frequency = tx_frequency * 10 + (*buf++ - '0');
	}
	buf++;
	while(*buf != ',')
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               rx_frequency = rx_frequency * 10 + (*buf++ - '0');
	}
        buf++;
	while((*buf != '\r') && (*buf != '\n'))
	{
		tx_power = tx_power * 10 + (*buf++ - '0');
	}

	cur_param.tx_freq = tx_frequency;
	cur_param.rx_freq = rx_frequency;
	cur_param.power = tx_power;
	if (0 == get_param_from_current_tx_freq(tx_frequency, &cur_param.tx_parm))
	{
		 memset(&cur_param.tx_parm, 0, sizeof(cur_param.tx_parm));
		 UARTprintf("There is no tx param at current tx freq\n\r");
	}
	else
	{
		cmd_che();
		UARTprintf("Config radio over\n\r");  //�ɹ����õ�̨
	}
}

/*ɾ��ָ���ķ���Ƶ�ʲ���*/
void proc_CFG_del_assign_send_freg_param(char *buf)       //0F
{
	uint8_t i;
	radio_param_t radio_param_temp;
	unsigned long assign_freq = 0;  
        
	while((*buf != '\r') && (*buf != '\n'))
	{
		if((*buf >'9')||(*buf <'0'))   
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               assign_freq = assign_freq * 10 + (*buf++ - '0');	/*�ַ�תʮ����*/
	}	

	for (i=0; i<NUM_FREQS; i++)
	{
		if (assign_freq == radio_param.tx_parm[i].freq)
		{	
			//����Flashʱ���Ȱ�Flash�����ڴ��У�Ȼ���޸����ݣ��ٲ������飬����̻ش�
			//�������������Ա����ͬһ�����ڵ���������Ĩ��
			memcpy(&radio_param_temp, &radio_param, sizeof(radio_param_temp));
			memset(&radio_param_temp.tx_parm[i], 0xff, sizeof(radio_param_temp.tx_parm[i]));	
			memcpy(&radio_param, &radio_param_temp, sizeof(radio_param_temp));
			bubble_tx_pram(radio_param.tx_parm, NUM_FREQS);
			radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
			return;
		}
	}
	UARTprintf("ɾ������Ƶ��: %u��������\n\r", assign_freq);
}

/*���浱ǰ����Ƶ�ʲ���*/
void proc_CFG_save_receive_freg_param(char *buf)   //0E���ݲ�֧��
{
	//UARTprintf("����������󣬲�֧�ָ�����\n");
	radio_param.rx_parm.midlvl = cur_param.rx_parm.midlvl;
	radio_param.rx_parm.range = cur_param.rx_parm.range;
	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
	UARTprintf("������ղ���:���ĵ�ƽ%u,���ֵ%u\n\r", radio_param.rx_parm.midlvl,radio_param.rx_parm.range);
}

/*���浱ǰ����Ƶ�ʲ���*/
void proc_CFG_save_current_send_freg_param(void)    //0D
{
	uint8_t i;
	uint8_t flag_same_freq = 0;  //�ñ���Ϊ1ʱ��ʾtx_pram���Ѵ�����ͬƵ�ʵ����ݣ���ʱд����������ԭ����
	uint8_t flag_space = 0;          //�ñ���Ϊ1ʱ��ʾtx_pram��δ���������Լ�����������

	/*����ѱ�������ͬƵ������ݣ������µ����ݸ���*/
	for (i=0; i<NUM_FREQS; i++)
	{
		if (cur_param.tx_parm.freq == radio_param.tx_parm[i].freq)
		{
			memcpy(&radio_param.tx_parm[i], &cur_param.tx_parm, sizeof(cur_param.tx_parm));
			flag_same_freq = 1;
			break;
		}
	}
	if (flag_same_freq == 1)
	{
		bubble_tx_pram(radio_param.tx_parm, NUM_FREQS);
		radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
		return;
	}
	
	/*����Ѵ���NUM_FREQS��Ƶ�㣬�򷵻ر������*/
	for (i=0; i<NUM_FREQS; i++)
	{
		if ((radio_param.tx_parm[i].freq == 0) || (radio_param.tx_parm[i].freq == (unsigned long)-1) )  flag_space = 1;
	}
	if (flag_space == 0)
	{
		UARTprintf("�Ѵ���Ƶ������\n");
		UARTprintf("��ȷ��Ҫ�������棬��������ɾ��һ��Ƶ������\n\r");
		return;
	}
	
	memcpy(&radio_param.tx_parm[NUM_FREQS-1], &cur_param.tx_parm, sizeof(cur_param.tx_parm));
	radio_param.flag = 1;
	bubble_tx_pram(radio_param.tx_parm, NUM_FREQS);
	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
}

/*���浱ǰ����������Ƶ�ʷ�Χ*/
void proc_CFG_save_current_pub_param(void)    //0C
{
	memcpy(&radio_param.freq_range, &cur_param.freq_range, sizeof(cur_param.freq_range));
	memcpy(&radio_param.pm, &cur_param.pm, sizeof(cur_param.pm));
	radio_param.flag = 1;

	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
}

/*���õ�̨Ƶ�ʷ�Χ*/
void proc_CFG_set_range_freg(char *buf)    //0B
{
	unsigned long value1=0;
       unsigned long value2=0;
      
	while(*buf != ',')
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value1 = value1*10 + (*buf++ - '0');
	}
        buf++;
	while((*buf != '\r') && (*buf != '\n'))
	{
		value2 = value2 * 10 + (*buf++ - '0');
	}

	cur_param.freq_range.start_freq = value1;
        cur_param.freq_range.end_freq = value2;
	radio_param.freq_range.start_freq = cur_param.freq_range.start_freq;
	radio_param.freq_range.end_freq = cur_param.freq_range.end_freq;   
}

/*��ӡ���в���*/
void proc_CFG_prin_all_param()      //0A
{

}

/*��ӡ��ǰ����Ƶ�ʵ����в���*/
void proc_CFG_prin_all_receive_freg_param()   //09���ݲ�֧��
{
	UARTprintf("��ǰ����Ƶ�ʵ����в�����ӡ����\n");
	UARTprintf("����Ƶ��%u, �����ź����ĵ�ƽ%u, �����źŷ���%u\n", 
		cur_param.tx_parm.freq, cur_param.rx_parm.midlvl,cur_param.rx_parm.range);
}

 /*��ӡ��ǰ����Ƶ�ʵ����в���*/
void proc_CFG_prin_cur_send_freg_param()    //08
{	
	UARTprintf("��ǰ����Ƶ�ʵ����в�����ӡ����\n");
	UARTprintf("����Ƶ��%u, �����ź����ĵ�ƽ%u, �����źŷ���%u, ��̨ƽ�����mod(1)%u, ��̨ƽ�����mod(2)%u\n", 
		cur_param.tx_parm.freq, cur_param.tx_parm.midlvl, cur_param.tx_parm.range, cur_param.tx_parm.mod1, cur_param.tx_parm.mod2);
	//UARTprintf("PRTT, %u, %u, %u, %u, %u\n", cur_param.tx_freq, cur_param.tx_parm.midlvl, 
	//	cur_param.tx_parm.range, cur_param.tx_parm.mod1, cur_param.tx_parm.mod2);
}

 /*���õ�̨���տ�խ��*/
void proc_CFG_rev_wb_nb_param(char *buf)   //07���ݲ�֧��
{
	UARTprintf("����������󣬲�֧�ָ�����\n");
  
        /*uint8_t value=0;
	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
        cur_param.n_w_band = value;*/
}
 
/*���õ�̨���书�ʲο���ƽ*/
void proc_CFG_send_ref_power_param(char *buf)    //06
{
	uint8_t value=0;
	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	cur_param.pm.tx_power_setting = value;
}

/*���õ�̨���侧�������ƽ*/
void proc_CFG_send_ref_ocsi_param(char *buf)   //05
{
	uint8_t value=0;
	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	cur_param.pm.tx_ref_osci = value;
}

/*���õ�̨ƽ��mod2*/
void proc_CFG_balance_mod2(char *buf)    //04
{
	uint8_t value = 0;

	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	cur_param.tx_parm.mod2 = value;
	//radio_param.tx_parm[NUM_FREQS-1].mod2 = cur_param.tx_parm.mod2;
}

/*���õ�̨ƽ��mod1*/
void proc_CFG_balance_mod1(char *buf)    //03
{
	uint8_t value = 0;

	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	cur_param.tx_parm.mod1 = value;
	//radio_param.tx_parm[NUM_FREQS-1].mod1 = cur_param.tx_parm.mod1;
}

/*���÷����źŷ���*/
void proc_CFG_send_range(char *buf)      //02
{
	uint8_t value = 0;
	while((*buf != '\r') && (*buf != '\n'))
	{
          if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	/*if (value > 128)
	{
		UARTprintf("%d����Ч����\n\r", value);
		return;
	}*/
	cur_param.tx_parm.range = value;
	//radio_param.tx_parm[NUM_FREQS-1].range= cur_param.tx_parm.range;
       iic_write(2,value);//���÷����źŷ���
}

/*���÷����ź����ĵ�ƽ*/
void proc_CFG_send_midlevel(char *buf)   //01
{
	uint32_t value = 0;
	
	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	if (value > 256)
	{
		UARTprintf("%d����Ч����\n\r", value);
		return;
	}       
	cur_param.tx_parm.midlvl = value;
	//radio_param.tx_parm[NUM_FREQS-1].midlvl = cur_param.tx_parm.midlvl;
	iic_write(3,value); /*�������ֵ�λ��3*/
}

/*���ý����ź����ĵ�ƽ*/
void proc_CFG_recv_midlevel(char *buf)
{  
	uint32_t value = 0;
	
	while((*buf != '\r') && (*buf != '\n'))
	{
                if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	if (value > 256)
	{
		UARTprintf("%d����Ч����\n\r", value);
		return;
	}       
	iic_write(0,value);

	cur_param.rx_parm.midlvl = value;
	UARTprintf("���ý����ź����ĵ�ƽ,%u\n\r", cur_param.rx_parm.midlvl);
}

/*���ý����źŷ���*/
void proc_CFG_recv_range(char *buf)   
{
	uint8_t value = 0;
	while((*buf != '\r') && (*buf != '\n'))
	{
          if((*buf >'9')||(*buf <'0'))
		{
			  UARTprintf("Input error\n\r");
			  return;
	        }
               value = value*10+(*buf++ -'0');	
	}
	/*if (value > 128)
	{
		UARTprintf("%d����Ч����\n\r", value);
		return;
	}*/

    iic_write(1,value);

	cur_param.rx_parm.range = value;
	UARTprintf("���ý����źŷ��ֵ,%u\n\r", cur_param.rx_parm.range);
}


void proc_CFG_write_SN(char *buf)	
{
	uint16_t i=0;
	
	
	while((buf[i] != '\r') && (buf[i] != '\n'))
	{
		cur_param.SN[i] = buf[i];
		i++;
		if(i>60)
		{	
			UARTprintf("����:����ʧ�ܣ�SN����,%s\n\r", buf);
			return;
		}
	}
	
	memcpy(&radio_param.SN, &cur_param.SN, sizeof(cur_param.SN));
	radio_param.flag = 1;
	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));

	UARTprintf("����ɹ���SN:%s\n\r", radio_param.SN);	
}

void proc_CFG_read_SN(void)
{
	UARTprintf("CFG,18,%s\r\n", radio_param.SN);		
}


