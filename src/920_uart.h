
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
    unsigned char switch_timer; //定时器开关标志 0:关 1:开
    unsigned char perdic_cnt;   //周期定时次数
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

/**存储DIAG1命令返回信息中的Alarm status、PA forward power、PA reflected power的值**/
typedef struct _DIAG_buf
{
        char alarm_status[MAX_SIZE];//电台故障信息缓存
        char pa_forwpwr[MAX_SIZE];  //前向功率缓存
        char pa_reflpwr[MAX_SIZE]; //反向功率缓存
        char pa_temperature[MAX_SIZE];//温度
        unsigned int alarm_sequence;  
        unsigned int forwpwr_sequence;
        unsigned int reflpwr_sequence;
	 unsigned int tempera_sequence;
	 
}DIAG_buf_t;

extern DIAG_buf_t resp_DIAG_buf;
extern int PT_DG_flag ;

/************************/

/*数据结构初始化*/
extern void init_radio_uart(void);

/*十进制转字符串*/
extern char *itoa(int num, char *str, int radix);

/*字符串转换成整数*/
extern int atoi(const char *str);

/*CHE命令*/
extern void cmd_che(void);

/*PT命令*/
extern void cmd_pt(void);

/*POTS?命令，读取电台中保存的PT命令四项参数*/
extern void cmd_pots_query(void);

extern void sent_pt(void);

/*读取POTS?命令的响应*/
extern void resp_cmd_pots_query(void);

/*数据结构resp_DIAG_buf初始化*/
extern void init_resp_DIAG_buf(void);
/*数据结构cur_param初始化*/
void init_cur_param(void);

/*向电台发送DIAG1等命令，获取电台故障、前向功率、反向功率等信息*/
extern void cmd_DIAGM_que(void);
extern void cmd_DIAG1_que(void);
extern void cmd_DIAG0_que(void);      //关闭
static void handle_cmd_DIAG1(const char *buf, unsigned char len);

extern void UART_920_tx_isr(void);	//920 UART发送中断服务程序
extern void UART_920_rx_isr(void);	//920 UART接收中断服务程序



/****************************/
#endif
