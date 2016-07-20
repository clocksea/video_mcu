/********************************************************************
版权所有	：Copyright (C) 2013 承联通信
文件名字	：PLL.c
文件功能	：配置PLL寄存器程序
创建日期	：2013-06-30
创建人		：Wuqy
修改标识	：
修改描述	：
*********************************************************************/
#include "includes.h"
#include "ssi.h"


R_LATCH PLL_R_latch;
N_LATCH PLL_N_latch;
FUNC_LATCH PLL_init_latch;
//FUNC_LATCH PLL_func_latch;


void ssi1_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_SSI1);		//使能SSI1模块
	
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);	//使能GPIOH
	
	GPIOPinConfigure(GPIO_PH4_SSI1CLK);
	GPIOPinConfigure(GPIO_PH6_SSI1RX);
	GPIOPinConfigure(GPIO_PH7_SSI1TX);
	
	GPIOPinTypeSSI(GPIO_PORTH_BASE,GPIO_PIN_4);
	GPIOPinTypeSSI(GPIO_PORTH_BASE,GPIO_PIN_6);
	GPIOPinTypeSSI(GPIO_PORTH_BASE,GPIO_PIN_7);
	
	//位速率:500K，数据宽度:8位，模式:SPO=0, SPH=0
	SSIConfigSetExpClk(	SSI1_BASE, SysCtlClockGet(), SSI_FRF_MOTO_MODE_0,
	                	SSI_MODE_MASTER, 500000, 8);
	
	SSIEnable(SSI1_BASE);							//使能SSI1
}


void PLL_chip_enable(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);	//使能GPIOH
	
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_0);
	GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_0, 0x01);	//使能PLL芯片
}


void PLL_LE_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOH);	//使能GPIOH
	
	GPIOPinTypeGPIOOutput(GPIO_PORTH_BASE, GPIO_PIN_1);
	GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_1, 0x00);
}


void config_a_latch(uint32_t data)
{
	uint32_t i = 0;
	
	GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_1, 0x00);	//LE go low
	for (i=0; i<10; i++) ;
	
	for (i = 0; i < SPI_BYTES; i++)
	{
		SSIDataPut(SSI1_BASE, ((uint8_t *)&data)[SPI_BYTES-1-i]);
	}
	
	while (SSIBusy(SSI1_BASE))
	{
	}
	
	for (i=0; i<10; i++) ;
	GPIOPinWrite(GPIO_PORTH_BASE, GPIO_PIN_1, 0x02);	//LE go high
}


void config_PLL(void)
{
	uint32_t i = 0;
	
	ssi1_init();
	PLL_chip_enable();
	PLL_LE_init();
	
	//Config Initialization Latch
	PLL_init_latch.bit.PD2 = 1;			//PD2 : SYNCHRONOUS POWER-DOWN
	PLL_init_latch.bit.CPI2 = 7;		//CPI2 : 5.0mA@4.7K
	PLL_init_latch.bit.CPI1 = 7;		//CPI1 : 5.0mA@4.7K
	PLL_init_latch.bit.TCC = 7;			//TIMER COUNTER CONTROL : 31 PFD CYCLES
	PLL_init_latch.bit.FL_mode = 1;		//FASTLOCK MODE : FASTLOCK MODE 2
	PLL_init_latch.bit.FL_EN = 0;		//FASTLOCK ENABLE : DISABLED
	PLL_init_latch.bit.CP_TS = 0;		//CHARGE PUMP OUTPUT : NORMAL
	PLL_init_latch.bit.PDP = 1;			//PHASE DETECTOR POLARITY : POSITIVE
	PLL_init_latch.bit.mux_out = 1;		//MUXOUT CONTROL : DIGITAL LOCK DETECT
	PLL_init_latch.bit.PD1 = 0;			//PD1 : NORMAL OPERATION
	PLL_init_latch.bit.cnt_rst = 0;		//COUNTER RESET : NORMAL
	PLL_init_latch.bit.ctrl_bits = 3;	//CONTROL BITS : Initialization Latch
	
	config_a_latch(PLL_init_latch.all);
	for (i=0; i<200; i++) ;
	
	//Config Reference Counter Latch
	PLL_R_latch.bit.LDP = 1;			//LOCK DETECT PRECISION : 5 CONSECUTIVE CYCLES
	PLL_R_latch.bit.test_mode = 0;		//TEST MODE BITS : NORMAL OPERATION
	PLL_R_latch.bit.ABW = 1;			//ANTIBACKLASH PULSE WIDTH : 1.3ns
	PLL_R_latch.bit.R_cnt = 100;		//REFERENCE COUNTER : 100
	PLL_R_latch.bit.ctrl_bits = 0;		//CONTROL BITS : R Counter
	
	config_a_latch(PLL_R_latch.all);
	for (i=0; i<200; i++) ;
	
	//Config N Counter Latch
	PLL_N_latch.bit.cp_gain = 0;		//CP GAIN : CPI1
	PLL_N_latch.bit.N_cnt = 100;		//N COUNTER : 100
	PLL_N_latch.bit.ctrl_bits = 1;		//CONTROL BITS : N Counter
	
	config_a_latch(PLL_N_latch.all);
}


void reconfig_PLL(uint32_t CPI1, uint32_t mux_out, uint32_t ref_cnt, uint32_t N_cnt)
{
	uint32_t i = 0;
	
	//Config Initialization Latch
	PLL_init_latch.bit.CPI1 = CPI1;		//CHARGE PUMP CURRENT SETTING 1
	PLL_init_latch.bit.mux_out = mux_out;	//MUXOUT
	config_a_latch(PLL_init_latch.all);
	for (i=0; i<200; i++) ;
	
	//Config Reference Counter Latch
	PLL_R_latch.bit.R_cnt = ref_cnt;	//REFERENCE COUNTER
	config_a_latch(PLL_R_latch.all);
	for (i=0; i<200; i++) ;
	
	//Config N Counter Latch
	PLL_N_latch.bit.N_cnt = N_cnt;	//N COUNTER
	config_a_latch(PLL_N_latch.all);
}


/*配置PLL相关参数*/
void proc_CFG_set_pll(char *buf)		//16
{
	uint32_t i = 0;
	uint32_t CPI1 = 0;				//CHARGE PUMP CURRENT SETTING 1
	uint32_t mux_out_sel = 0;		//MUXOUT
	uint32_t ref_cnt = 0;			//REFERENCE COUNTER
	uint32_t N_cnt = 0;				//N COUNTER
	
	while (*buf != ',')
	{
		if ((*buf >'9')||(*buf <'0'))
		{
			UARTprintf("Input error\n\r");
			return;
		}
		CPI1 = CPI1 * 10 + (*buf++ - '0');
		i++;
	}
	if (CPI1 > 7)
	{
		UARTprintf("%d是无效参数，CPI1可配范围为0~7\n\r", CPI1);
		return;
	}
	
	if (i == 0)
		CPI1 = PLL_init_latch.bit.CPI1;	//未配置则采用之前的参数
	else
		i = 0;
	
	buf++;
	
	while (*buf != ',')
	{
		if ((*buf >'9')||(*buf <'0'))
		{
			UARTprintf("Input error\n\r");
			return;
		}
		mux_out_sel = mux_out_sel * 10 + (*buf++ - '0');
		i++;
	}
	if (mux_out_sel > 7)
	{
		UARTprintf("%d是无效参数，mux_out可配范围为0~7\n\r", mux_out_sel);
		return;
	}
	
	if (i == 0)
		mux_out_sel = PLL_init_latch.bit.mux_out;	//未配置则采用之前的参数
	else
		i = 0;
	
	buf++;
	
	while (*buf != ',')
	{
		if ((*buf >'9')||(*buf <'0'))
		{
			UARTprintf("Input error\n\r");
			return;
		}
		ref_cnt = ref_cnt * 10 + (*buf++ - '0');
		i++;
	}
	
	if (i == 0)
		ref_cnt = PLL_R_latch.bit.R_cnt;	//未配置则采用之前的参数
	else
		i = 0;
	
	if ((ref_cnt > 16383) || (ref_cnt == 0))
	{
		UARTprintf("%d是无效参数，ref_cnt可配范围为1~16383\n\r", ref_cnt);
		return;
	}
	
	buf++;
	
	while ((*buf != '\r') && (*buf != '\n'))
	{
		if ((*buf >'9')||(*buf <'0'))
		{
			UARTprintf("Input error\n\r");
			return;
		}
		N_cnt = N_cnt * 10 + (*buf++ - '0');
		i++;
	}
	
	if (i == 0)
		N_cnt = PLL_N_latch.bit.N_cnt;	//未配置则采用之前的参数
	
	if ((N_cnt > 8191) || (N_cnt == 0))
	{
		UARTprintf("%d是无效参数，N_cnt可配范围为1~8191\n\r", N_cnt);
		return;
	}
	
	reconfig_PLL(CPI1, mux_out_sel, ref_cnt, N_cnt);
	UARTprintf("CPI1:%d; mux_out:%d; ref_cnt:%d; N_cnt:%d\n\r", CPI1,mux_out_sel,ref_cnt,N_cnt);
}
