
#include "includes.h"

#define  PARA_LEN  20

#define	 INIT	0 
#define  PT   	1	
#define	 DG		2
#define  MX9XX   3   /*INFO? : ��ѯ*/


#define SS_IDLE			0
#define SS_T_IDLE		1
#define SS_CR_RECV		2
#define SS_BEGINRECV	3
#define SS_SYN      	4
#define	CMD_CCUHBREP	5




/*���Դ������ݽṹ��ʼ��*/
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


/*ʮ����ת�ַ���*/
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

/*�ַ���ת��������*/
int atoi(const char *str)
{
	 //assert(str);                    //�����δ�����κ��ַ�����һ����Ч�ַ������˳�����
	 int total = 0;                     //Ҫ����Ľ��
	 int sign = 1;                      //���ֵķ���
	 
	 if(*str == '-')
	 {                                      //�����һ���ָ���
		  sign = -1;                 //���Ϊ����
		  str++;                     //������һ���ַ�
	 }
	 else if(*str == '+')           //�����һ���ַ�������
	 {
	   	str++;                       //�޸Ķ���������һ���ַ�
	 }
	 
	 while(*str)                      //���ַ�����Ϊ��β
	 {
		  unsigned int ch = *str - '0';    //�����ַ������������
		  //assert(ch<=9 && ch>=0);       //��������ֲ���0��9�ķ�Χ���˳�����
		  total = total * 10 + ch;           //ԭ��������һλ
		  str++;                                 //�ƶ�ָ�뵽��һ���ַ�
	 }
	 return total * sign;                      //�������õ����������������
}



/***************************************************************/
/*920 UART�����жϷ������*/
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
/*920 UART�����жϷ������*/
void UART_920_rx_isr(void)
{
#if 0
      unsigned char tmp;
      unsigned short tail;
	while (UARTCharsAvail(uart_for_920))                            //������FIFO���п�������
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
/*UART0�жϷ������*/
void UART0IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART0_BASE, true);                 //��ȡ�ж�״̬
    UARTIntClear(UART0_BASE, ulStatus);                         //����ж�״̬

	if (board_ver == 1)				//Ϊ0x1���ӦCCU/PC����
	{
		if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //�����жϻ���ճ�ʱ�ж�
			UART_CCU_rx_isr();
	}
	else if (board_ver == 0xF)		//Ϊ0xF���Ӧ920����
	{
		if (ulStatus & UART_INT_TX)                             //�����ж�
			UART_920_tx_isr();
		
		if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //�����жϻ���ճ�ʱ�ж�
			UART_920_rx_isr();
	}
}


//CCU/PC UART�����жϷ������
void UART_CCU_rx_isr(void)
{
	while (UARTCharsAvail(uart_for_ccu))
	{
		ccu_uart_buf.ring_buf[ccu_uart_buf.ring_tail] = (unsigned char)UARTCharGetNonBlocking(uart_for_ccu);	
		ccu_uart_buf.ring_tail = (ccu_uart_buf.ring_tail + 1) % MAX_COM_RING_PACKSIZE;
	}
}

//UART1�жϷ������
void UART1IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART1_BASE, true);                   //��ȡ�ж�״̬
    UARTIntClear(UART1_BASE, ulStatus);                              //����ж�״̬     

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //�����жϻ���ճ�ʱ�ж�
		UART_CCU_rx_isr();
}


