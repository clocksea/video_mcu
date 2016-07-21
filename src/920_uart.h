
#ifndef  _920_UART_H
#define  _920_UART_H

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


#define MX920           0x10
#define MX930           0x11

extern unsigned char flag_query_cops;

typedef struct 
{
    unsigned char switch_timer; //��ʱ�����ر�־ 0:�� 1:��
    unsigned char perdic_cnt;   //���ڶ�ʱ����
}timer_param_t;

typedef struct 
{
    unsigned char mx9xx;  //0x10: mx920, 0x11: mx930
    timer_param_t timer;
	uint32_t type_getted;
	uint32_t info_cmd_sended;
}radio_version_info_t;

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

/**�洢DIAG1�������Ϣ�е�Alarm status��PA forward power��PA reflected power��ֵ**/
typedef struct _DIAG_buf
{
        char alarm_status[MAX_SIZE];//��̨������Ϣ����
        char pa_forwpwr[MAX_SIZE];  //ǰ���ʻ���
        char pa_reflpwr[MAX_SIZE]; //�����ʻ���
        char pa_temperature[MAX_SIZE];//�¶�
        unsigned int alarm_sequence;  
        unsigned int forwpwr_sequence;
        unsigned int reflpwr_sequence;
	 unsigned int tempera_sequence;
	 
}DIAG_buf_t;

extern DIAG_buf_t resp_DIAG_buf;
extern int PT_DG_flag ;

/************************/

/*���ݽṹ��ʼ��*/
extern void init_radio_uart(void);

/*ʮ����ת�ַ���*/
extern char *itoa(int num, char *str, int radix);

/*�ַ���ת��������*/
extern int atoi(const char *str);

/*CHE����*/
extern void cmd_che(void);

/*PT����*/
extern void cmd_pt(void);

/*POTS?�����ȡ��̨�б����PT�����������*/
extern void cmd_pots_query(void);

extern void sent_pt(void);

/*��ȡPOTS?�������Ӧ*/
extern void resp_cmd_pots_query(void);

/*���ݽṹresp_DIAG_buf��ʼ��*/
extern void init_resp_DIAG_buf(void);
/*���ݽṹcur_param��ʼ��*/
void init_cur_param(void);

/*���̨����DIAG1�������ȡ��̨���ϡ�ǰ���ʡ������ʵ���Ϣ*/
extern void cmd_DIAGM_que(void);
extern void cmd_DIAG1_que(void);
extern void cmd_DIAG0_que(void);      //�ر�
static void handle_cmd_DIAG1(const char *buf, unsigned char len);

extern void UART_920_tx_isr(void);	//920 UART�����жϷ������
extern void UART_920_rx_isr(void);	//920 UART�����жϷ������



/****************************/
#endif
