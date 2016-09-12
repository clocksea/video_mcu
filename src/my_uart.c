#include <stdarg.h>
#include "includes.h"


extern uart_info_t  g_uart_dbg;
extern uart_info_t  g_uart_comm;

/*���Դ������ݽṹ��ʼ��*/
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


/*ͨ�ô��ڷ��ͺ������������̨��������*/
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
	while (UARTCharsAvail(base))                            //������FIFO���п�������
	{
		uart->ring_buf[uart->ring_tail++] = (unsigned char)UARTCharGetNonBlocking(base);
		if(uart->ring_tail>=MAX_COM_RING_PACKSIZE)
			uart->ring_tail=0;	
	}
}

/***************************************************************/
/*UART0�жϷ������*/
void UART0IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART0_BASE, true);                 //��ȡ�ж�״̬
    UARTIntClear(UART0_BASE, ulStatus);                         //����ж�״̬

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)		//�����жϻ���ճ�ʱ�ж�
		UART_rx_isr(UART0_BASE,&g_uart_comm);
	if (ulStatus & UART_INT_TX) 							//�����ж�
		UART_tx_isr(UART0_BASE,&g_uart_comm);

}


//UART1�жϷ������
void UART1IntHandler(void)                             
{
    unsigned long ulStatus;
    ulStatus = UARTIntStatus(UART1_BASE, true);                   //��ȡ�ж�״̬
    UARTIntClear(UART1_BASE, ulStatus);                              //����ж�״̬     

	if (ulStatus&UART_INT_RX || ulStatus&UART_INT_RT)       //�����жϻ���ճ�ʱ�ж�
		UART_rx_isr(UART1_BASE,&g_uart_dbg);
	if (ulStatus & UART_INT_TX)                             //�����ж�
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


