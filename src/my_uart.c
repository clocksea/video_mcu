#include <stdarg.h>
#include "includes.h"


extern uart_info_t  g_uart_dbg;
extern uart_info_t  g_uart_comm;

/*调试串口数据结构初始化*/
void init_uart_info(uart_info_t * info)
{
	info->ring_head = 0;
	info->ring_tail = 0;
	info->stream_state = STATE_IDLE;
	info->com_rx_sequence = 0;
	info->com_received_flg = FALSE;
	info->com_send_ready_flg = TRUE;
	info->com_tx_len = 0;
	info->com_tx_sequence = 0;
	memset(info->com_tx_buf,0,sizeof(info->com_tx_buf));
	memset(info->com_rx_buf,0,sizeof(info->com_rx_buf));
	memset(info->ring_buf,0,sizeof(info->ring_buf));	
}


/*十进制转字符串*/
char *itoa(int num, char *str, int radix)   
{
	char string[] = "0123456789ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	char* ptr = str;
	int i;
	int j;

	while (num)
	{
		*ptr++ = string[num % radix];
		num /= radix;
		if (num < radix)
		{
			*ptr++ = string[num];
			*ptr = '\0';
			break;
		}
	}

	j = ptr - str - 1;

	for (i = 0; i < (ptr - str) / 2; i++)
	{
		int temp = str[i];
		str[i] = str[j];
		str[j--] = temp;
	}

	return str;
}

/*字符串转换成整数*/
int atoi(const char *str)
{
	 //assert(str);                    //如果并未输入任何字符串或一个无效字符串则退出运行
	 int total = 0;                     //要输出的结果
	 int sign = 1;                      //数字的符号
	 
	 if(*str == '-')
	 {                                      //如果第一个字负号
		  sign = -1;                 //输出为负数
		  str++;                     //移至下一个字符
	 }
	 else if(*str == '+')           //如果第一个字符是正号
	 {
	   	str++;                       //无改动，移至下一个字符
	 }
	 
	 while(*str)                      //当字符串还为结尾
	 {
		  unsigned int ch = *str - '0';    //当下字符所代表的数字
		  //assert(ch<=9 && ch>=0);       //如果该数字不再0至9的范围内退出运行
		  total = total * 10 + ch;           //原数字增加一位
		  str++;                                 //移动指针到下一个字符
	 }
	 return total * sign;                      //返还所得到的整数，加入符号
}


/*通用串口发送函数，例如向电台发送命令*/
void uart_send_datas(unsigned long uart_base,uart_info_t *uart_data,char *data_2_send)
{
	unsigned short i, len;
	
	while(uart_data->com_send_ready_flg == FALSE);
	strcpy(uart_data->com_tx_buf,data_2_send);
	uart_data->com_tx_len = len = strlen(data_2_send);
	if(len >= UART_BYE_FIFO)  len = UART_BYE_FIFO;
	for(i=0; i< len; i++)
	{
		UARTCharPutNonBlocking(uart_base, uart_data->com_tx_buf[i]);
	} 
    
	uart_data->com_tx_sequence = len;
	uart_data->com_send_ready_flg = FALSE;   	
}



void UART_tx_isr(uint32_t base,uart_info_t *uart)
{
	unsigned char i, len;

	if (uart->com_send_ready_flg == FALSE)
	{
		if (uart->com_tx_len == uart->com_tx_sequence)
			uart->com_send_ready_flg = TRUE;
		else
		{
			if (uart->com_tx_len < (uart->com_tx_sequence + UART_BYE_FIFO))
				len = uart->com_tx_len - uart->com_tx_sequence;
			else
				len = UART_BYE_FIFO;
			
			for (i= 0 ; i< len ; i++)
			{
				UARTCharPutNonBlocking(base, uart->com_tx_buf[i + uart->com_tx_sequence]);
			}
			uart->com_tx_sequence += len;
		}
	}
}


void UART_rx_isr(uint32_t base,uart_info_t *uart)
{
	while (UARTCharsAvail(base))                            //若接收FIFO中有可用数据
	{
		uart->ring_buf[uart->ring_tail++] = (unsigned char)UARTCharGetNonBlocking(base);
		if(uart->ring_tail>=MAX_COM_RING_PACKSIZE)
			uart->ring_tail=0;	
	}
}

/***************************************************************/
/*UART0中断服务程序*/
void UART0IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART0_BASE, true);                 //读取中断状态
    UARTIntClear(UART0_BASE, ulStatus);                         //清除中断状态

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)		//接收中断或接收超时中断
		UART_rx_isr(UART0_BASE,&g_uart_comm);
	if (ulStatus & UART_INT_TX) 							//发送中断
		UART_tx_isr(UART0_BASE,&g_uart_comm);

}


//UART1中断服务程序
void UART1IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART1_BASE, true);                   //读取中断状态
    UARTIntClear(UART1_BASE, ulStatus);                              //清除中断状态     

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //接收中断或接收超时中断
		UART_rx_isr(UART1_BASE,&g_uart_dbg);
	if (ulStatus & UART_INT_TX)                             //发送中断
		UART_tx_isr(UART1_BASE,&g_uart_dbg);
}


void myprintf(const char *pcString, ...)
{
	char tmp[MAX_COMM_PACKSIZE];
    va_list vaArgP;

	memset(tmp,0,sizeof(tmp));
    //
    // Start the varargs processing.
    //
    va_start(vaArgP, pcString);	
	
	vsnprintf(tmp,sizeof(tmp), pcString, vaArgP);

    //
    // End the varargs processing.
    //
    va_end(vaArgP);	

	uart_send_datas(UART1_BASE,&g_uart_dbg,tmp);
	
}


void printf_to_android(const char *pcString, ...)
{
	char tmp[MAX_COMM_PACKSIZE];
    va_list vaArgP;

	memset(tmp,0,sizeof(tmp));
    //
    // Start the varargs processing.
    //
    va_start(vaArgP, pcString);	
	
	vsnprintf(tmp,sizeof(tmp), pcString, vaArgP);

    //
    // End the varargs processing.
    //
    va_end(vaArgP);	

	uart_send_datas(UART0_BASE,&g_uart_comm,tmp);	
}


