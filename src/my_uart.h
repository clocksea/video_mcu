
#ifndef  _MY_UART_H_
#define  _MY_UART_H_

#ifndef	TRUE
#define 	TRUE		1
#endif

#ifndef	FALSE
#define 	FALSE		0
#endif
#define  LF  0x0A
#define CR 0x0D

#define	UART_BYE_FIFO		14
#ifndef  MAX_COM_RING_PACKSIZE
#define  MAX_COM_RING_PACKSIZE  (512)
#endif

#ifndef  MAX_COMM_PACKSIZE
#define  MAX_COMM_PACKSIZE          (128)
#endif

#ifndef  MAX_SIZE
#define  MAX_SIZE          (20)
#endif

#define SS_IDLE			0
#define SS_T_IDLE		1
#define SS_CR_RECV		2
#define SS_BEGINRECV	3
#define SS_SYN      	4
#define	CMD_CCUHBREP	5


typedef struct 
{
    unsigned char switch_timer; //��ʱ�����ر�־ 0:�� 1:��
    unsigned char perdic_cnt;   //���ڶ�ʱ����
}timer_param_t;


typedef struct
{
	//RX
	 char	         com_rx_buf[MAX_COMM_PACKSIZE];
	unsigned char   ring_buf[MAX_COM_RING_PACKSIZE];
	unsigned short  ring_head;
	unsigned short  ring_tail;
	unsigned char	  stream_state;
	unsigned short  com_rx_sequence;
	unsigned char   com_received_flg;
	unsigned short  com_rx_len;
	
	//TX
	char	                com_tx_buf[MAX_COMM_PACKSIZE];
	unsigned char	  com_send_ready_flg;
	unsigned short  com_tx_len;
	unsigned short  com_tx_sequence;

}uart_info_t;


/*���ݽṹ��ʼ��*/
extern void init_uart_info(uart_info_t * info);

/*ʮ����ת�ַ���*/
extern char *itoa(int num, char *str, int radix);

/*�ַ���ת��������*/
extern int atoi(const char *str);

extern void UART_920_tx_isr(void);	//920 UART�����жϷ������
extern void UART_920_rx_isr(void);	//920 UART�����жϷ������



/****************************/
#endif

