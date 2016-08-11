
#include "includes.h"





unsigned int c_timer;
int heart_flag;
int alarm_flag;	//��̨�澯��־λ����IO �ж���λ
int	alarm_count;//alarm_status ���ʹ���
 
int32_t watchdog_cnt=0;


uart_info_t  g_uart_dbg;/*���Դ���*/
uart_info_t  g_uart_comm;/*ͨ�Ŵ���*/


extern pll_info_t pll_info[2];

//ϵͳʱ�ӳ�ʼ��
void clockInit(void)
{
    SysCtlClockSet(SYSCTL_USE_OSC |                 //  ϵͳʱ������
                   SYSCTL_OSC_MAIN |                       //  ����������
                   SYSCTL_XTAL_16_3MHZ |                //  ���16.3(16.384)MHz����
                   SYSCTL_SYSDIV_1);                        //  ��Ƶϵ��Ϊ1��������Ƶ
}

//LED��ʼ��
static void led_init(void)
{
	SysCtlPeripheralEnable(LED_PERIPH);                          //ʹ��LED���ڵ�GPIO�˿�
	GPIOPinTypeGPIOOutput(LED_PORT, LED_PIN);            //����LED���ڹܽ�Ϊ���
	GPIOPinWrite(LED_PORT, LED_PIN, 0x00);                   //����LED
}

//��תLED
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

//UART0��ʼ�����ͺ��İ�����

void uart0Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART0);            //ʹ��UART0ģ��
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);            //ʹ��RX/TX���ڵ�GPIO�˿�

	GPIOPinConfigure(GPIO_PA0_U0RX);                              //ѡ��UART0ʹ�õĹܽ�
	GPIOPinConfigure(GPIO_PA1_U0TX);

	GPIOPinTypeUART(GPIO_PORTA_BASE,                          //����RX/TX���ڹܽ�Ϊ
	                GPIO_PIN_0 | GPIO_PIN_1);                         //UART�շ�����

	UARTConfigSetExpClk(UART0_BASE, SysCtlClockGet(),   //����UART�˿�
	             57600,                                                           //�����ʣ�57600
	             UART_CONFIG_WLEN_8 |                                //����λ��8
	             UART_CONFIG_STOP_ONE |                            //ֹͣλ��1
	             UART_CONFIG_PAR_NONE);                            //û��У��

	UARTFIFOLevelSet(UART0_BASE,                                   //�����շ�FIFO�жϴ������
	               UART_FIFO_TX1_8,                                      //����FIFOΪ1/8���(14B)
	               UART_FIFO_RX7_8);                                    //����FIFOΪ7/8��ȣ�14B��


	IntPrioritySet(INT_UART0, 1<<5);                                 //����UART0�ж����ȼ�Ϊ1
	UARTIntEnable(UART0_BASE, UART_INT_RX | UART_INT_RT);   //ʹ��UART0�Ľ����жϺͳ�ʱ�ж�
	
	IntEnable(INT_UART0);                                                //ʹ��UART0���ж�

	UARTEnable(UART0_BASE);                                          //ʹ��UART�˿�
}

//UART1��ʼ�������Դ���
void uart1Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_UART1);            //ʹ��UART1ģ��
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            //ʹ��RX/TX���ڵ�GPIO�˿�
	
	GPIOPinConfigure(GPIO_PD0_U1RX);                              //ѡ��UART0ʹ�õĹܽ�
	GPIOPinConfigure(GPIO_PD1_U1TX);

	GPIOPinTypeUART(GPIO_PORTD_BASE,                          //����RX/TX���ڹܽ�Ϊ
	                GPIO_PIN_0 | GPIO_PIN_1);                          //UART�շ�����

	UARTConfigSetExpClk(UART1_BASE, SysCtlClockGet(),   //����UART�˿�
	              57600,                                                           //�����ʣ�57600
	              UART_CONFIG_WLEN_8 |                                //����λ��8
	              UART_CONFIG_STOP_ONE |                            //ֹͣλ��1
	              UART_CONFIG_PAR_NONE);                            //û��У��
	              
	UARTFIFOLevelSet(UART1_BASE,                                   //�����շ�FIFO�жϴ������
	                 UART_FIFO_TX1_8,                                      //����FIFOΪ1/8���(14B)
	                 UART_FIFO_RX7_8);                                    //����FIFOΪ7/8��ȣ�14B��
	

	IntPrioritySet(INT_UART1, 0<<5);                                 //����UART1�ж����ȼ�Ϊ0
	UARTIntEnable(UART1_BASE, UART_INT_TX | UART_INT_RX | UART_INT_RT);   //ʹ��UART1�ķ����ж�/�����ж�/���ճ�ʱ�ж�
	
	IntEnable(INT_UART1);                                                //ʹ��UART1���ж�
	
	UARTEnable(UART1_BASE);                                         //ʹ��UART�˿�
}

//IIC��ʼ��
void iic_init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2C0);               //ʹ��iic0ģ��  
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            //ʹ��iic0���ڵ�GPIO�˿�
	GPIOPinConfigure(GPIO_PB2_I2C0SCL);                       
	GPIOPinConfigure(GPIO_PB3_I2C0SDA);  
	GPIOPinTypeI2C(GPIO_PORTB_BASE,                            //����iic0���ڹܽ�Ϊ
	                GPIO_PIN_2 | GPIO_PIN_3);                       //iic����               
	I2CMasterInitExpClk(I2C0_MASTER_BASE, SysCtlClockGet(), false); //MCU��ʼ��ΪI2C��������������Ϊ100kbps
	I2CMasterSlaveAddrSet(I2C0_MASTER_BASE, 0x50, false);   //false=write,ture=read,1010000B
}

//iis�ӿڳ�ʼ��
void iis_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_I2S0);               //ʹ��iis0ģ��  
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOF);            //ʹ��iis0���ڵ�GPIO�˿�
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOD);            //ʹ��iis0���ڵ�GPIO�˿�
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




//adc�ӿڳ�ʼ��
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


 
//��ʱ����ʼ��
void timer1Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER1);                                    //ʹ��timer1ģ��
	
	TimerConfigure(TIMER1_BASE, TIMER_CFG_32_BIT_PER);                         //timer1���ó�32λ���δ���
	TimerLoadSet(TIMER1_BASE, TIMER_A, SysCtlClockGet());                       //��ʱʱ��Ϊ1s

	IntPrioritySet(INT_TIMER1A, 6<<5);                                                       //����timer1�ж����ȼ�Ϊ6
	TimerIntEnable(TIMER1_BASE, TIMER_TIMA_TIMEOUT);                           //ʹ��timer1��ʱ�ж�
	IntEnable(INT_TIMER1A);                                                                       //ʹ��timer1�ж�
	TimerEnable(TIMER1_BASE, TIMER_A);                                                   //ʹ��timer1����
}

//Timer1�жϷ������
void timer1AIntHandler(void)                             
{
	unsigned long ulStatus;
	ulStatus = TimerIntStatus(TIMER1_BASE, true);                 //��ȡ�ж�״̬
	TimerIntClear(TIMER1_BASE, ulStatus);                            //����ж�״̬
	if (ulStatus & TIMER_TIMA_TIMEOUT)                               //timer0A��ʱ�ж�
	{
		GPIOPinWrite(LED_PORT, LED_PIN, ~GPIOPinRead(LED_PORT, LED_PIN) ); 	
	}
}

void timer3Init()
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER3);                                    //ʹ��timer1ģ��
	
	TimerConfigure(TIMER3_BASE, TIMER_CFG_32_BIT_PER);                         //timer1���ó�32λ���δ���
	TimerLoadSet(TIMER3_BASE, TIMER_A, SysCtlClockGet()<<1);                       //��ʱʱ��Ϊ2s

	IntPrioritySet(INT_TIMER3A, 6<<5);                                                       //����timer1�ж����ȼ�Ϊ6
	TimerIntEnable(TIMER3_BASE, TIMER_TIMA_TIMEOUT);                           //ʹ��timer1��ʱ�ж�
	IntEnable(INT_TIMER3A);                                                                       //ʹ��timer1�ж�
	TimerEnable(TIMER3_BASE, TIMER_A);      
}

void timer3AIntHandler(void)  
{
#if 0  
    unsigned long ulStatus;
    char buf[32] = {0};
    ulStatus = TimerIntStatus(TIMER3_BASE, true);                 //��ȡ�ж�״̬
    TimerIntClear(TIMER3_BASE, ulStatus);                            //����ж�״̬
    
    if (ulStatus & TIMER_TIMA_TIMEOUT) 
    {
    	strcat(buf , "INFO?");                   
    	strcat(buf , "\r");                              //������"\r"
        uart_send_datas(uart_for_920,&g_uart_dbg,buf);
		g_radio_station.info_cmd_sended = 1;
		//UARTprintf("->920����:%s\n\r",buf);      //������	
    }    
	if(g_radio_station.type_getted)
    {
        TimerDisable(TIMER3_BASE, TIMER_A);      
    }
#endif    
}


//GPIO�жϷ������
void mygpio(void)
{
	alarm_flag = 1;	//��̨�澯
}


//GPIOA�жϳ�ʼ��
void GPIOAIntHandler_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);//��ʼ������GPIOA

	//GPIOPinTypeGPIOInput(GPIO_PORTA_BASE,GPIO_PIN_3);	// ���ùܽ� 3��Ϊ���룬���������
	GPIODirModeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_DIR_MODE_IN);		//����Ϊ����˿�
	GPIOPadConfigSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_STRENGTH_4MA,	//4mA���������ǿ��
					 GPIO_PIN_TYPE_STD_WPU);							//������

	GPIOIntTypeSet(GPIO_PORTA_BASE, GPIO_PIN_3, GPIO_FALLING_EDGE); //�½��ش���
	IntPrioritySet(INT_GPIOA, 4<<5);							 //����GPIO�ж����ȼ�Ϊ4
	GPIOPinIntEnable(GPIO_PORTA_BASE, GPIO_PIN_3);		   //ʹ����Ӧ�ܽŵ��ж�
	IntEnable(INT_GPIOA);
	
	//IntMasterEnable(); // ʹ�ܴ������ж�
}


//GPIOA�жϳ���
void GPIOAIntHandler(void)
{
	long ulStatus;
	//unsigned char int_status,i;
    ulStatus = GPIOPinIntStatus(GPIO_PORTA_BASE, true);            //��ȡ�ж�״̬
   	GPIOPinIntClear(GPIO_PORTA_BASE, ulStatus);                    //����ж�״̬
	if (ulStatus & GPIO_PIN_3)                                     //�����PA3�ж�
	{
		mygpio();
	}
}


//GPIOE�жϳ�ʼ��
void GPIOEIntHandler_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOE);//��ʼ������GPIOE

	//GPIOPinTypeGPIOInput(GPIO_PORTE_BASE,GPIO_PIN_4);	// ���ùܽ� 4��Ϊ���룬���������
	GPIODirModeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_DIR_MODE_IN);		//����Ϊ����˿�
	GPIOPadConfigSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_STRENGTH_4MA,	//4mA���������ǿ��
					 GPIO_PIN_TYPE_STD_WPU);							//������

	GPIOIntTypeSet(GPIO_PORTE_BASE, GPIO_PIN_4, GPIO_FALLING_EDGE); //�½��ش���
	IntPrioritySet(INT_GPIOE, 4<<5);							 //����GPIO�ж����ȼ�Ϊ4
	GPIOPinIntEnable(GPIO_PORTE_BASE, GPIO_PIN_4);		   //ʹ����Ӧ�ܽŵ��ж�
	IntEnable(INT_GPIOE);
	
	//IntMasterEnable(); // ʹ�ܴ������ж�
}


//GPIOE�жϳ���
void GPIOEIntHandler(void)
{
	long ulStatus;
	//unsigned char int_status,i;
    ulStatus = GPIOPinIntStatus(GPIO_PORTE_BASE, true);            //��ȡ�ж�״̬
   	GPIOPinIntClear(GPIO_PORTE_BASE, ulStatus);                    //����ж�״̬
	if (ulStatus & GPIO_PIN_4)                                     //�����PE4�ж�
	{
		mygpio();
	}
}



//��ʼ��ʹ��920���ڵ�RS232��ƽת��оƬ
void radio_uart_init(void)
{
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOA);
	GPIOPinTypeGPIOOutput(GPIO_PORTA_BASE, GPIO_PIN_4 | GPIO_PIN_5);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_4, 0x10);
	GPIOPinWrite(GPIO_PORTA_BASE, GPIO_PIN_5, 0x00);
}

void timer2Init(void)
{   
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER2);                                    //ʹ��timer1ģ��
	
	TimerConfigure(TIMER2_BASE, TIMER_CFG_32_BIT_PER);        //timer1���ó�32λ
	TimerLoadSet(TIMER2_BASE, TIMER_BOTH, SysCtlClockGet());                     
	
	IntPrioritySet(INT_TIMER2A, 7<<5);                                                       //����timer1�ж����ȼ�Ϊ6
	TimerIntEnable(TIMER2_BASE, TIMER_TIMA_TIMEOUT);                           //ʹ��timer1��ʱ�ж�
	IntEnable(INT_TIMER2A);                                                                       //ʹ��timer1�ж�
	TimerEnable(TIMER2_BASE, TIMER_A);      //ʹ��timer1����

}


void timer2AIntHandler(void)
{
	unsigned long ulStatus;
	ulStatus = TimerIntStatus(TIMER2_BASE, true);                 //��ȡ�ж�״̬
	TimerIntClear(TIMER2_BASE, ulStatus);                            //����ж�״̬
	if (ulStatus & TIMER_TIMA_TIMEOUT)                               //timer0A��ʱ�ж�
	{
		GPIOPinWrite(LED_PORT, LED_PIN, ~GPIOPinRead(LED_PORT, LED_PIN)); 	
	}

}

void timer0Init(void)
{
	//�ṩ����ʱ��
	SysCtlPeripheralEnable(SYSCTL_PERIPH_GPIOB);
	SysCtlPeripheralEnable(SYSCTL_PERIPH_TIMER0);
	//���ø�������
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
	ulStatus = WatchdogIntStatus(WATCHDOG0_BASE, true);                 //��ȡ�ж�״̬
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


