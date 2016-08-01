/********************************************************************
版权所有	：Copyright (C) 2011 承联通信
文件名字	：配置电台参数程序
文件功能	：配置电台参数
创建日期	：2011-06-09
创建人		：ZhongH
修改标识	：
修改描述	：
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
	FlashUsecSet(SysCtlClockGet() / 1000000);   //设置每us的CPU时钟数
	memset(&ccu_pc_buf.buf,0,sizeof(ccu_pc_buf.buf));
	memset(&ccu_uart_buf,0,sizeof(ccu_uart_buf));
	memset(&radio_param,0,sizeof(radio_param));
	memset(&cur_param,0,sizeof(cur_param));
}

/*iic写操作*/
void iic_write(uint8_t addr,uint8_t data)
{
  	  // 将要发送的字符放置到数据寄存器中 
	  I2CMasterDataPut(I2C0_MASTER_BASE,addr);   // 启动将字符从主机发送到从机。 
	  I2CMasterControl(I2C0_MASTER_BASE,I2C_MASTER_CMD_BURST_SEND_START); // 延时一段时间，直至发送完成。
	  while(I2CMasterBusy(I2C0_MASTER_BASE));    // 将要发送的字符放置到数据寄存器中。 
	  I2CMasterDataPut(I2C0_MASTER_BASE,data);   // 启动将字符从主机发送到从机。
	  I2CMasterControl(I2C0_MASTER_BASE,I2C_MASTER_CMD_BURST_SEND_FINISH);  // 延时一段时间，直至发送完成。 
	  while(I2CMasterBusy(I2C0_MASTER_BASE)); 
}

/*从flash中读取数据*/
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
                return 0; 					/*表示没有参数*/
       }
	else		
	{
		tx_freq = tx_param->freq ;
		return 1;
	}	
}

/*将radio_param写入flash*/
/*pdata: 指向数据缓冲区的指针*/
/*addr: (addr+FLASH_BASE_ADDR)为编程的起始地址，必须是4的倍数*/
/*size: 编程的字节数，必须是4的倍数*/
static void radio_param_write_to_flash(unsigned long *pdata, unsigned long addr, unsigned long size)
{
	if (FlashErase(FLASH_BASE_ADDR) != 0)  //擦除1K大小的Flash，Flash起始地址为FLASH_BASE_ADDR
	{
		UARTprintf("Flash擦除出错，数据未能写入\n\r");      
		return;
	}
	//if (FlashProgram((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param))) 
	if (FlashProgram(pdata, addr, size)) 
	{
		UARTprintf("Flash编程出错，数据未能写入\n\r");      
		return;
	}
}


/*用冒泡法对tx_pram[]进行升序排列*/
/*buf: 指向需要排序的数据，n: 需要排序的数据个数*/
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

/*读MCU软件版本*/
void read_mcu_version(void)
{
	UARTprintf("MCU软件版本号 = %s\n\r", mcusoft_version);
}

/*仅配置电台平衡参数*/
void proc_CFG_pt(void)
{
	cmd_pt();
}


/*仅配置电台发射频率，接收频率和发射功率*/
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
		UARTprintf("Config radio over\n\r");  //成功配置电台
	}
}

/*删除指定的发射频率参数*/
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
               assign_freq = assign_freq * 10 + (*buf++ - '0');	/*字符转十进制*/
	}	

	for (i=0; i<NUM_FREQS; i++)
	{
		if (assign_freq == radio_param.tx_parm[i].freq)
		{	
			//擦除Flash时，先把Flash读到内存中，然后修改内容，再擦除区块，最后编程回存
			//这样操作，可以避免把同一区块内的其他数据抹掉
			memcpy(&radio_param_temp, &radio_param, sizeof(radio_param_temp));
			memset(&radio_param_temp.tx_parm[i], 0xff, sizeof(radio_param_temp.tx_parm[i]));	
			memcpy(&radio_param, &radio_param_temp, sizeof(radio_param_temp));
			bubble_tx_pram(radio_param.tx_parm, NUM_FREQS);
			radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
			return;
		}
	}
	UARTprintf("删除出错，频率: %u并不存在\n\r", assign_freq);
}

/*保存当前接收频率参数*/
void proc_CFG_save_receive_freg_param(char *buf)   //0E，暂不支持
{
	//UARTprintf("命令输入错误，不支持该命令\n");
	radio_param.rx_parm.midlvl = cur_param.rx_parm.midlvl;
	radio_param.rx_parm.range = cur_param.rx_parm.range;
	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
	UARTprintf("保存接收参数:中心电平%u,峰峰值%u\n\r", radio_param.rx_parm.midlvl,radio_param.rx_parm.range);
}

/*保存当前发射频率参数*/
void proc_CFG_save_current_send_freg_param(void)    //0D
{
	uint8_t i;
	uint8_t flag_same_freq = 0;  //该变量为1时表示tx_pram中已存在相同频率的数据，此时写操作将覆盖原数据
	uint8_t flag_space = 0;          //该变量为1时表示tx_pram尚未存满，可以继续保存数据

	/*如果已保存有相同频点的数据，则用新的数据覆盖*/
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
	
	/*如果已存满NUM_FREQS个频点，则返回保存出错*/
	for (i=0; i<NUM_FREQS; i++)
	{
		if ((radio_param.tx_parm[i].freq == 0) || (radio_param.tx_parm[i].freq == (unsigned long)-1) )  flag_space = 1;
	}
	if (flag_space == 0)
	{
		UARTprintf("已存满频点数据\n");
		UARTprintf("若确认要继续保存，请至少先删掉一条频点数据\n\r");
		return;
	}
	
	memcpy(&radio_param.tx_parm[NUM_FREQS-1], &cur_param.tx_parm, sizeof(cur_param.tx_parm));
	radio_param.flag = 1;
	bubble_tx_pram(radio_param.tx_parm, NUM_FREQS);
	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
}

/*保存当前公共参数和频率范围*/
void proc_CFG_save_current_pub_param(void)    //0C
{
	memcpy(&radio_param.freq_range, &cur_param.freq_range, sizeof(cur_param.freq_range));
	memcpy(&radio_param.pm, &cur_param.pm, sizeof(cur_param.pm));
	radio_param.flag = 1;

	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));
}

/*设置电台频率范围*/
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

/*打印所有参数*/
void proc_CFG_prin_all_param()      //0A
{

}

/*打印当前接收频率的所有参数*/
void proc_CFG_prin_all_receive_freg_param()   //09，暂不支持
{
	UARTprintf("当前接收频率的所有参数打印如下\n");
	UARTprintf("接收频率%u, 接收信号中心电平%u, 接收信号幅度%u\n", 
		cur_param.tx_parm.freq, cur_param.rx_parm.midlvl,cur_param.rx_parm.range);
}

 /*打印当前发射频率的所有参数*/
void proc_CFG_prin_cur_send_freg_param()    //08
{	
	UARTprintf("当前发射频率的所有参数打印如下\n");
	UARTprintf("发射频率%u, 发射信号中心电平%u, 发射信号幅度%u, 电台平衡参数mod(1)%u, 电台平衡参数mod(2)%u\n", 
		cur_param.tx_parm.freq, cur_param.tx_parm.midlvl, cur_param.tx_parm.range, cur_param.tx_parm.mod1, cur_param.tx_parm.mod2);
	//UARTprintf("PRTT, %u, %u, %u, %u, %u\n", cur_param.tx_freq, cur_param.tx_parm.midlvl, 
	//	cur_param.tx_parm.range, cur_param.tx_parm.mod1, cur_param.tx_parm.mod2);
}

 /*配置电台接收宽窄带*/
void proc_CFG_rev_wb_nb_param(char *buf)   //07，暂不支持
{
	UARTprintf("命令输入错误，不支持该命令\n");
  
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
 
/*配置电台发射功率参考电平*/
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

/*配置电台发射晶振参数电平*/
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

/*配置电台平衡mod2*/
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

/*配置电台平衡mod1*/
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

/*配置发射信号幅度*/
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
		UARTprintf("%d是无效参数\n\r", value);
		return;
	}*/
	cur_param.tx_parm.range = value;
	//radio_param.tx_parm[NUM_FREQS-1].range= cur_param.tx_parm.range;
       iic_write(2,value);//配置发射信号幅度
}

/*配置发射信号中心电平*/
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
		UARTprintf("%d是无效参数\n\r", value);
		return;
	}       
	cur_param.tx_parm.midlvl = value;
	//radio_param.tx_parm[NUM_FREQS-1].midlvl = cur_param.tx_parm.midlvl;
	iic_write(3,value); /*配置数字电位器3*/
}

/*配置接收信号中心电平*/
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
		UARTprintf("%d是无效参数\n\r", value);
		return;
	}       
	iic_write(0,value);

	cur_param.rx_parm.midlvl = value;
	UARTprintf("配置接收信号中心电平,%u\n\r", cur_param.rx_parm.midlvl);
}

/*配置接收信号幅度*/
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
		UARTprintf("%d是无效参数\n\r", value);
		return;
	}*/

    iic_write(1,value);

	cur_param.rx_parm.range = value;
	UARTprintf("配置接收信号峰峰值,%u\n\r", cur_param.rx_parm.range);
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
			UARTprintf("错误:保存失败，SN过长,%s\n\r", buf);
			return;
		}
	}
	
	memcpy(&radio_param.SN, &cur_param.SN, sizeof(cur_param.SN));
	radio_param.flag = 1;
	radio_param_write_to_flash((unsigned long *)&radio_param, FLASH_BASE_ADDR, sizeof(radio_param));

	UARTprintf("保存成功，SN:%s\n\r", radio_param.SN);	
}

void proc_CFG_read_SN(void)
{
	UARTprintf("CFG,18,%s\r\n", radio_param.SN);		
}


