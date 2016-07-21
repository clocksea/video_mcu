
#include "includes.h"

#define  PARA_LEN  20

#define	 INIT	0 
#define  PT   	1	
#define	 DG		2
#define  MX9XX   3   /*INFO? : 查询*/


#define SS_IDLE			0
#define SS_T_IDLE		1
#define SS_CR_RECV		2
#define SS_BEGINRECV	3
#define SS_SYN      	4
#define	CMD_CCUHBREP	5




/*调试串口数据结构初始化*/
void init_uart_info(uart_info_t * info)
{
	info->ring_head = 0;
	info->ring_tail = 0;
	info->stream_state = SS_SYN;
	info->com_rx_sequence = 0;
	info->com_received_flg = FALSE;
	info->com_send_ready_flg = TRUE;
	info->com_tx_len = 0;
	info->com_tx_sequence = 0;	
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



/***************************************************************/
/*920 UART发送中断服务程序*/
void UART_920_tx_isr(void)
{
	unsigned char i, len;
#if 0
	if (g_uart_dbg.com_send_ready_flg == FALSE)
	{
		if (g_uart_dbg.com_tx_len == g_uart_dbg.com_tx_sequence)
			g_uart_dbg.com_send_ready_flg = TRUE;
		else
		{
			if (g_uart_dbg.com_tx_len < (g_uart_dbg.com_tx_sequence + UART_BYE_FIFO))
				len = g_uart_dbg.com_tx_len - g_uart_dbg.com_tx_sequence;
			else
				len = UART_BYE_FIFO;
			
			for (i= 0 ; i< len ; i++)
			{
				UARTCharPutNonBlocking(uart_for_920, g_uart_dbg.com_tx_buf[i + g_uart_dbg.com_tx_sequence]);
			}
			g_uart_dbg.com_tx_sequence += len;
		}
	}
#endif 
}

/***************************************************************/
/*920 UART接收中断服务程序*/
void UART_920_rx_isr(void)
{
#if 0
      unsigned char tmp;
      unsigned short tail;
	while (UARTCharsAvail(uart_for_920))                            //若接收FIFO中有可用数据
	{
                  g_uart_dbg.ring_buf[g_uart_dbg.ring_tail] = (unsigned char)UARTCharGetNonBlocking(uart_for_920);
		  g_uart_dbg.ring_tail = (g_uart_dbg.ring_tail + 1) % MAX_COM_RING_PACKSIZE; 
               // tmp=(unsigned char)UARTCharGetNonBlocking(uart_for_920);
              //  tail = g_uart_dbg.ring_tail+1;
              //  if(tail==MAX_COM_RING_PACKSIZE)
              //    tail=0;
              //  if(tail!=g_uart_dbg.ring_head)
              //  {
     		//  g_uart_dbg.ring_buf[g_uart_dbg.ring_tail] = tmp;
		//  g_uart_dbg.ring_tail = (g_uart_dbg.ring_tail + 1) % MAX_COM_RING_PACKSIZE;           
               // }
                  


	}
#endif
}

/***************************************************************/
/*UART0中断服务程序*/
void UART0IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART0_BASE, true);                 //读取中断状态
    UARTIntClear(UART0_BASE, ulStatus);                         //清除中断状态

	if (board_ver == 1)				//为0x1则对应CCU/PC串口
	{
		if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //接收中断或接收超时中断
			UART_CCU_rx_isr();
	}
	else if (board_ver == 0xF)		//为0xF则对应920串口
	{
		if (ulStatus & UART_INT_TX)                             //发送中断
			UART_920_tx_isr();
		
		if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //接收中断或接收超时中断
			UART_920_rx_isr();
	}
}


//CCU/PC UART接收中断服务程序
void UART_CCU_rx_isr(void)
{
	while (UARTCharsAvail(uart_for_ccu))
	{
		ccu_uart_buf.ring_buf[ccu_uart_buf.ring_tail] = (unsigned char)UARTCharGetNonBlocking(uart_for_ccu);	
		ccu_uart_buf.ring_tail = (ccu_uart_buf.ring_tail + 1) % MAX_COM_RING_PACKSIZE;
	}
}

//UART1中断服务程序
void UART1IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART1_BASE, true);                   //读取中断状态
    UARTIntClear(UART1_BASE, ulStatus);                              //清除中断状态     

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //接收中断或接收超时中断
		UART_CCU_rx_isr();
}


