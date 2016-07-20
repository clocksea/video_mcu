#ifndef __RADIO_H__
#define __RADIO_H__

#ifndef  MAX_COM_RING_PACKSIZE
#define  MAX_COM_RING_PACKSIZE  (512)
#endif

#ifndef  MAX_COMM_PACKSIZE
#define  MAX_COMM_PACKSIZE          (128)
#endif

typedef unsigned char uint8_t;
typedef unsigned short uint16_t;
typedef unsigned int uint32_t;

typedef  char int8_t;
typedef  short int16_t;
typedef  int int32_t;

typedef struct{
	uint8_t flag;
	volatile uint16_t cnt;
	int8_t buf[64];
}ccu_pc_buf_t;

typedef struct _uart_buf_t
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
}uart_buf_t;

#define STATE_IDLE    0                  //空闲状态
#define STATE_HEAD  1
#define STATE_HEAD_DOT  3
#define STATE_REV_START 4
#define STATE_REV_END     5

 
#define  NUM_FREQS  (3)   //宏NUM_FREQS指代需保存多少个频点的参数


/*每个发射频率对应的参数*/
typedef struct ccu_stored_tx_parm
{
  unsigned long freq;   /*发射频率*/
  uint32_t midlvl;  /*发射信号中心电平，调整的数字电位器值，0-127*/
  uint32_t range;  /*发射信号幅度，调整的数字电位器值，0-127*/
  uint32_t mod1;   /*电台平衡参数mod1：Balance pot setting*/
  uint32_t mod2;   /*电台平衡参数mod2：Deviation pot setting*/
}ccu_stored_tx_parm_t;

/*电台公共参数*/
typedef struct radio_public_parm
{
  uint32_t tx_ref_osci;            /*发射晶振参数电平*/
  uint32_t tx_power_setting;  /*发射功率参考电平*/
}radio_public_parm_t;

/*电台频率范围*/
typedef struct radio_freq_size
{
  unsigned long start_freq;
  unsigned long end_freq;
}radio_freq_size_t;


/*typedef struct ccu_send_parm
{
   unsigned long tx_freq;
   unsigned long rx_freq;
   uint32_t power;
}ccu_send_parm_t; */

typedef struct{
	uint32_t midlvl;
	uint32_t range;
}rx_parm_t;

/*电台参数（存于flash中）*/
typedef struct {
	uint32_t flag;                                  /*表示有没有参数，1表示有，非1表示没有*/
	//uint32_t tx_freq;                          /*发射频率*/
 	radio_freq_size_t freq_range;        /*频率范围*/
 	radio_public_parm_t pm;                 /*公共参数*/
	rx_parm_t rx_parm;
 	ccu_stored_tx_parm_t tx_parm[NUM_FREQS]; /*全频道保存3个频点的参数*/
	char SN[64];
}radio_param_t;

/*电台参数（当前）*/
typedef struct {
	  uint32_t power;                      /*发射功率*/
	  uint32_t n_w_band;                /*接收宽窄带*/
	  uint32_t tx_freq;                    /*发射频率*/
	  uint32_t rx_freq;                    /*接收频率*/
 	radio_freq_size_t freq_range;   /*频率范围*/
 	radio_public_parm_t pm;            /*公共参数*/
	rx_parm_t rx_parm;/*当前发送参数*/
 	ccu_stored_tx_parm_t tx_parm; /*当前频点的参数*/
	char SN[64];
}cur_param_t;


//extern radio_param_t radio_param;
//extern uint8_t count;
extern ccu_pc_buf_t ccu_pc_buf;
extern uart_buf_t ccu_uart_buf;
extern cur_param_t cur_param;

extern uint32_t radio_strlen(char *s);
extern void init_ccu_buf(void);
extern void iic_write(uint8_t addr,uint8_t data);
extern uint32_t radio_strlen(char *s);
extern uint16_t get_param_from_current_tx_freq(uint32_t tx_freq, ccu_stored_tx_parm_t *tx_param);
extern void get_stored_param_from_flash(void);
extern void bubble_tx_pram(ccu_stored_tx_parm_t *buf, uint32_t n);

extern void proc_CFG_send_midlevel(char *buf);    //01，配置发射信号中心电平
extern void proc_CFG_send_range(char *buf);         //02，配置发射信号幅度
extern void proc_CFG_balance_mod1(char *buf);    //03，配置电台平衡mod1
extern void proc_CFG_balance_mod2(char *buf);    //04，配置电台平衡mod2
extern void proc_CFG_send_ref_ocsi_param(char *buf);                   //05，配置电台发射晶振参数电平
extern void proc_CFG_send_ref_power_param(char *buf);               //06，配置电台发射功率参考电平
extern void proc_CFG_rev_wb_nb_param(char *buf);                         //07，配置电台接收宽窄带，暂不支持
extern void proc_CFG_prin_cur_send_freg_param(void);                //08，打印当前发射频率的所有参数
extern void proc_CFG_prin_all_receive_freg_param(void);              //09，打印当前接收频率的所有参数，暂不支持
extern void proc_CFG_prin_all_param(void);                                        //0A，打印所有参数
extern void proc_CFG_set_range_freg(char *buf);                                 //0B，设置电台频率范围
extern void proc_CFG_save_current_pub_param(void);                    //0C，保存当前公共参数和频率范围
extern void proc_CFG_save_current_send_freg_param(void);        //0D，保存当前发射频率参数
extern void proc_CFG_save_receive_freg_param(char *buf);             //0E，保存当前接收频率参数，暂不支持
extern void proc_CFG_del_assign_send_freg_param(char *buf);     //0F，删除指定的发射频率参数
extern void proc_CFG_tx_rx_power(char *buf);                                      //10，仅配置电台发射频率，接收频率和发射功率
extern void proc_CFG_pt(void);                                                                 //11，仅配置电台平衡参数
extern void read_mcu_version(void);                                                       //12，读MCU软件版本
extern  void proc_CFG_recv_midlevel(char *buf);
extern void proc_CFG_recv_range(char *buf);
#endif // __DEBUG_H__