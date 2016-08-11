
#include "includes.h"





unsigned int c_timer;
int heart_flag;
int alarm_flag;	//电台告警标志位，由IO 中断置位
int	alarm_count;//alarm_status 发送次数
 
int32_t watchdog_cnt=0;


uart_info_t  g_uart_dbg;/*调试串口*/
uart_info_t  g_uart_comm;/*通信串口*/


extern pll_info_t pll_info[2];

//系统时钟初始化
void clockInit(void)
{
    SysCtlClockSet(SYSCTL_USE_OSC |                 //  系统时钟设置
                   SYSCTL_OSC_MAIN |                       //  采用主振荡器
                   SYSCTL_XTAL_16_3MHZ |                //  外接16.3(16.384)MHz晶振
                   SYSCTL_SYSDIV_1);                        //  分频系数为1，即不分频
}

//LED初始化
static void led_init(void)
{
	SysCtlPeripheralEnable(LED_PERIPH);                          //使能LED所在的GPIO端口
	GPIOPinTypeGPIOOutput(LED_PORT, LED_PIN);            //设置LED所在管脚为输出
	GPIOPinWrite(LED_PORT, LED_PIN, 0x00);                   //点亮LED
}

//反转LED
static void led_retrieve()
{
	uint8_t value;
	static uint32_t i = 0;
	i++;
	if (i == 50000)
	{
		i = 0;
		value = GPIOPinRead(LED_PORT, LED_PIN);
		GPIOPinWrite(LED_PORT, LED_PIN, ~value);
	}
}

//UART0初始化，和核心板连接

void uart0Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);            //使能UART0模块
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);            //使能RX/TX所在的GPIO端口

	GPIOPinConfigure(GPIO_PA0_U0RX);                              //选择UART0使用的管脚
	GPIOPinConfigure(GPIO_PA1_U0TX);

	GPIOPinTypeUART(GPIO_PORTA_BASE,                          //配置RX/TX所在管脚为
	                GPIO_PIN_0 | GPIO_PIN_1);                         //UART收发功能

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(),   //配置UART端口
	             57600,                                                           //波特率：57600
	             UART_CONFIG_WLEN_8 |                                //数据位：8
	             UART_CONFIG_STOP_ONE |                            //停止位：1
	             UART_CONFIG_PAR_NONE);                            //没有校验

	UARTFIFOLevelSet(UART0_BASE,                                   //设置收发FIFO中断触发深度
	               UART_FIFO_TX1_8,                                      //发送FIFO为1/8深度(14B)
	               UART_FIFO_RX7_8);                                    //接收FIFO为7/8深度（14B）


	IntPrioritySet(INT_UART0, 1<<5);                                 //设置UART0中断优先级为1
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);   //使能UART0的接收中断和超时中断
	
	IntEnable(INT_UART0);                                                //使能UART0的中断

	UARTEnable(UART0_BASE);                                          //使能UART端口
}

//UART1初始化，调试串口
void uart1Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);            //使能UART1模块
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            //使能RX/TX所在的GPIO端口
	
	GPIOPinConfigure(GPIO_PD0_U1RX);                              //选择UART0使用的管脚
	GPIOPinConfigure(GPIO_PD1_U1TX);

	GPIOPinTypeUART(GPIO_PORTD_BASE,                          //配置RX/TX所在管脚为
	                GPIO_PIN_0 | GPIO_PIN_1);                          //UART收发功能

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(),   //配置UART端口
	              57600,                                                           //波特率：57600
	              UART_CONFIG_WLEN_8 |                                //数据位：8
	              UART_CONFIG_STOP_ONE |                            //停止位：1
	              UART_CONFIG_PAR_NONE);                            //没有校验
	              
	UARTFIFOLevelSet(UART1_BASE,                                   //设置收发FIFO中断触发深度
	                 UART_FIFO_TX1_8,                                      //发送FIFO为1/8深度(14B)
	                 UART_FIFO_RX7_8);                                    //接收FIFO为7/8深度（14B）
	

	IntPrioritySet(INT_UART1, 0<<5);                                 //设置UART1中断优先级为0
	UARTIntEnable(UART1_BASE, UART_INT_TX | UART_INT_RX | UART_INT_RT);   //使能UART1的发送中断/接收中断/接收超时中断
	
	IntEnable(INT_UART1);                                                //使能UART1的中断
	
	UARTEnable(UART1_BASE);                                         //使能UART端口
}

//IIC初始化
void iic_init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);               //使能iic0模块  
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            //使能iic0所在的GPIO端口
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);                       
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);  
	GPIOPinTypeI2C(GPIO_PORTB_BASE,                            //配置iic0所在管脚为
	                GPIO_PIN_2 | GPIO_PIN_3);                       //iic功能               
	I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false); //MCU初始化为I2C主机，传输速率为100kbps
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50, false);   //false=write,ture=read,1010000B
}

//iis接口初始化
void iis_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2S0);               //使能iis0模块  
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            //使能iis0所在的GPIO端口
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            //使能iis0所在的GPIO端口
	SysCtlI2SMClkSet(0, 48000 * 32 * 2 * 4);
	
	GPIOPinConfigure(GPIO_PD6_I2S0TXSCK);                       
	GPIOPinConfigure(GPIO_PF0_I2S0TXSD);  
	GPIOPinTypeI2S(GPIO_PORTD_BASE,GPIO_PIN_6); 
	GPIOPinTypeI2S(GPIO_PORTF_BASE,GPIO_PIN_0);
	
	I2SMasterClockSelect(I2S0_BASE,I2S_TX_MCLK_INT);
	I2STxConfigSet(I2S0_BASE,I2S_CONFIG_FORMAT_I2S|I2S_CONFIG_SCLK_INVERT|I2S_CONFIG_MODE_MONO|I2S_CONFIG_CLK_MASTER
							 |I2S_CONFIG_SAMPLE_SIZE_32|I2S_CONFIG_WIRE_SIZE_32|I2S_CONFIG_EMPTY_ZERO);
}


void pll_gpio_init(void)
{
	SysCtlPeripheralEnable(PLLLE_1_PERIPH);                      
	GPIOPinTypeGPIOOutput(PLLLE_1_PORT, PLLLE_1_PIN);       
	CLR_LE1();

	SysCtlPeripheralEnable(PLLCE_1_PERIPH);                      
	GPIOPinTypeGPIOOutput(PLLCE_1_PORT, PLLCE_1_PIN);       
	GPIOPinWrite(PLLCE_1_PORT, PLLCE_1_PIN, GPIO_PIN_0);

	SysCtlPeripheralEnable(PLLPDBRF_1_PERIPH);                      
	GPIOPinTypeGPIOOutput(PLLPDBRF_1_PORT, PLLPDBRF_1_PIN);       
	GPIOPinWrite(PLLPDBRF_1_PORT, PLLPDBRF_1_PIN, GPIO_PIN_6);	

	SysCtlPeripheralEnable(PLLCLK_PERIPH);						
	GPIOPinTypeGPIOOutput(PLLCLK_PORT, PLLCLK_PIN); 
	CLR_SCL();

	SysCtlPeripheralEnable(PLLDATA_PERIPH);						
	GPIOPinTypeGPIOOutput(PLLDATA_PORT, PLLDATA_PIN); 
	CLR_DATA();


	
}




void adcIntHandler(void)
{	
	unsigned long ulValue;
	
	while(ADCSequenceDataGet(ADC_BASE, 0, &ulValue));
	
}




//adc接口初始化
void adc_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC0);
	//SysCtlPeripheralEnable(SYSCTL_PERIPH_ADC1);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);  
	
	GPIOPinTypeADC(GPIO_PORTD_BASE,GPIO_PIN_2); 
	GPIOPinTypeADC(GPIO_PORTD_BASE,GPIO_PIN_4);

	//
	// Enable the first sample sequence to capture the value of channel 0 when
	// the processor trigger occurs.
	//
	ADCHardwareOversampleConfigure(ADC_BASE,64);
	ADCSequenceConfigure(ADC_BASE, 0, ADC_TRIGGER_PROCESSOR, 0);
	ADCSequenceStepConfigure(ADC_BASE, 0, 0,
	ADC_CTL_IE | ADC_CTL_END | ADC_CTL_CH7|ADC_CTL_CH13);
	ADCSequenceEnable(ADC_BASE, 0);
	
	ADCIntRegister(ADC_BASE,0,adcIntHandler);
	ADCIntEnable(ADC_BASE,0);

	//
	// Trigger the sample sequence.
	//
	ADCProcessorTrigger(ADC_BASE, 0);

	

}


 
//定时器初始化
void timer1Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);                                    //使能timer1模块
	
	TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);                         //timer1配置成32位单次触发
	TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet());                       //定时时间为1s

	IntPrioritySet(INT_TIMER1A, 6<<5);                                                       //设置timer1中断优先级为6
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);                           //使能timer1超时中断
	IntEnable(INT_TIMER1A);                                                                       //使能timer1中断
	TimerEnable(TIMER1_BASE, TIMER_A);                                                   //使能timer1计数
}

//Timer1中断服务程序
void timer1AIntHandler(void)                             
{
	unsigned long ulStatus;
	ulStatus = TimerIntStatus(TIMER1_BASE, true);                 //读取中断状态
	TimerIntClear(TIMER1_BASE, ulStatus);                            //清除中断状态
	if (ulStatus & TIMER_TIMA_TIMEOUT)                               //timer0A超时中断
	{
		GPIOPinWrite(LED_PORT, LED_PIN, ~GPIOPinRead(LED_PORT, LED_PIN) ); 	
	}
}

void timer3Init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);                                    //使能timer1模块
	
	TimerConfigure(TIMER3_BASE, TIMER_CFG_32_BIT_PER);                         //timer1配置成32位单次触发
	TimerLoadSet(TIMER3_BASE, TIMER_A, SysCtlClockGet()<<1);                       //定时时间为2s

	IntPrioritySet(INT_TIMER3A, 6<<5);                                                       //设置timer1中断优先级为6
	TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);                           //使能timer1超时中断
	IntEnable(INT_TIMER3A);                                                                       //使能timer1中断
	TimerEnable(TIMER3_BASE, TIMER_A);      
}

void timer3AIntHandler(void)  
{
#if 0  
    unsigned long ulStatus;
    char buf[32] = {0};
    ulStatus = TimerIntStatus(TIMER3_BASE, true);                 //读取中断状态
    TimerIntClear(TIMER3_BASE, ulStatus);                            //清除中断状态
    
    if (ulStatus & TIMER_TIMA_TIMEOUT) 
    {
    	strcat(buf , "INFO?");                   
    	strcat(buf , "\r");                              //结束符"\r"
        uart_send_datas(uart_for_920,&g_uart_dbg,buf);
		g_radio_station.info_cmd_sended = 1;
		//UARTprintf("->920命令:%s\n\r",buf);      //调试用	
    }    
	if(g_radio_station.type_getted)
    {
        TimerDisable(TIMER3_BASE, TIMER_A);      
    }
#endif    
}


//GPIO中断服务程序
void mygpio(void)
{
	alarm_flag = 1;	//电台告警
}


//GPIOA中断初始化
void GPIOAIntHandler_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);//初始化外设GPIOA

	//GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_3);	// 设置管脚 3作为输入，由软件控制
	GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_DIR_MODE_IN);		//设置为输入端口
	GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_STRENGTH_4MA,	//4mA的输出驱动强度
					 GPIO_PIN_TYPE_STD_WPU);							//弱上拉

	GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE); //下降沿触发
	IntPrioritySet(INT_GPIOA, 4<<5);							 //设置GPIO中断优先级为4
	GPIOPinIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);		   //使能相应管脚的中断
	IntEnable(INT_GPIOA);
	
	//IntMasterEnable(); // 使能处理器中断
}


//GPIOA中断程序
void GPIOAIntHandler(void)
{
	long ulStatus;
	//unsigned char int_status,i;
    ulStatus = GPIOPinIntStatus(GPIO_PORTA_BASE, true);            //读取中断状态
   	GPIOPinIntClear(GPIO_PORTA_BASE, ulStatus);                    //清除中断状态
	if (ulStatus & GPIO_PIN_3)                                     //如果是PA3中断
	{
		mygpio();
	}
}


//GPIOE中断初始化
void GPIOEIntHandler_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);//初始化外设GPIOE

	//GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,GPIO_PIN_4);	// 设置管脚 4作为输入，由软件控制
	GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);		//设置为输入端口
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA,	//4mA的输出驱动强度
					 GPIO_PIN_TYPE_STD_WPU);							//弱上拉

	GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE); //下降沿触发
	IntPrioritySet(INT_GPIOE, 4<<5);							 //设置GPIO中断优先级为4
	GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_4);		   //使能相应管脚的中断
	IntEnable(INT_GPIOE);
	
	//IntMasterEnable(); // 使能处理器中断
}


//GPIOE中断程序
void GPIOEIntHandler(void)
{
	long ulStatus;
	//unsigned char int_status,i;
    ulStatus = GPIOPinIntStatus(GPIO_PORTE_BASE, true);            //读取中断状态
   	GPIOPinIntClear(GPIO_PORTE_BASE, ulStatus);                    //清除中断状态
	if (ulStatus & GPIO_PIN_4)                                     //如果是PE4中断
	{
		mygpio();
	}
}



//初始化使能920串口的RS232电平转换芯片
void radio_uart_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0x10);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0x00);
}

void timer2Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);                                    //使能timer1模块
	
	TimerConfigure(TIMER2_BASE, TIMER_CFG_32_BIT_PER);        //timer1配置成32位
	TimerLoadSet(TIMER2_BASE, TIMER_BOTH, SysCtlClockGet());                     
	
	IntPrioritySet(INT_TIMER2A, 7<<5);                                                       //设置timer1中断优先级为6
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);                           //使能timer1超时中断
	IntEnable(INT_TIMER2A);                                                                       //使能timer1中断
	TimerEnable(TIMER2_BASE, TIMER_A);      //使能timer1计数

}


void timer2AIntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = TimerIntStatus(TIMER2_BASE, true);                 //读取中断状态
	TimerIntClear(TIMER2_BASE, ulStatus);                            //清除中断状态
	if (ulStatus & TIMER_TIMA_TIMEOUT)                               //timer0A超时中断
	{
		GPIOPinWrite(LED_PORT, LED_PIN, ~GPIOPinRead(LED_PORT, LED_PIN)); 	
	}

}

void timer0Init(void)
{
	//提供外设时钟
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	//配置复用引脚
	GPIOPinConfigure(GPIO_PB0_CCP0);
	GPIOPinTypeTimer(GPIO_PORTB_BASE,GPIO_PIN_0);
	TimerControlStall(TIMER0_BASE,TIMER_A,true);
	TimerConfigure(TIMER0_BASE,TIMER_CFG_16_BIT_PAIR | TIMER_CFG_A_CAP_COUNT);
	TimerControlEvent(TIMER0_BASE,TIMER_A,TIMER_EVENT_NEG_EDGE);
	TimerLoadSet(TIMER0_BASE,TIMER_A,0x8000);
	TimerEnable(TIMER0_BASE, TIMER_A);      
}


void WATCHDOGIntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = WatchdogIntStatus(WATCHDOG0_BASE, true);                 //读取中断状态
	WatchdogIntClear(WATCHDOG0_BASE);
	WatchdogReloadSet(WATCHDOG0_BASE,0xFEEFEE);
	GPIOPinWrite(LED_PORT, LED_PIN, ~GPIOPinRead(LED_PORT, LED_PIN) ); 

	watchdog_cnt++;
}


void watchdog_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_WDOG0);
	if(WatchdogLockState(WATCHDOG0_BASE) == true)
	{
		WatchdogUnlock(WATCHDOG0_BASE);
	}
	WatchdogReloadSet(WATCHDOG0_BASE,0xFEEFEE);
	WatchdogResetEnable(WATCHDOG0_BASE);
	WatchdogIntEnable(WATCHDOG0_BASE);
	WatchdogIntRegister(WATCHDOG0_BASE,WATCHDOGIntHandler);
	WatchdogEnable(WATCHDOG0_BASE);
}




void main(void)
{
	IntMasterDisable(); 

	clockInit();
	led_init();
	timer1Init();
	uart0Init();
	uart1Init();

	init_uart_info(&g_uart_comm);
	init_uart_info(&g_uart_dbg);

	//iis_init();
	//ssi0_init();
	pll_gpio_init();
	
	IntMasterEnable();  

	adf4351_init(1,&pll_info[0]);
	
	while(1)
	{
		proc_uart_buf(&g_uart_comm);
		proc_uart_buf(&g_uart_dbg);
	}
}


