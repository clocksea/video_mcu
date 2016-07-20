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

#define STATE_IDLE    0                  //����״̬
#define STATE_HEAD  1
#define STATE_HEAD_DOT  3
#define STATE_REV_START 4
#define STATE_REV_END     5

 
#define  NUM_FREQS  (3)   //��NUM_FREQSָ���豣����ٸ�Ƶ��Ĳ���


/*ÿ������Ƶ�ʶ�Ӧ�Ĳ���*/
typedef struct ccu_stored_tx_parm
{
  unsigned long freq;   /*����Ƶ��*/
  uint32_t midlvl;  /*�����ź����ĵ�ƽ�����������ֵ�λ��ֵ��0-127*/
  uint32_t range;  /*�����źŷ��ȣ����������ֵ�λ��ֵ��0-127*/
  uint32_t mod1;   /*��̨ƽ�����mod1��Balance pot setting*/
  uint32_t mod2;   /*��̨ƽ�����mod2��Deviation pot setting*/
}ccu_stored_tx_parm_t;

/*��̨��������*/
typedef struct radio_public_parm
{
  uint32_t tx_ref_osci;            /*���侧�������ƽ*/
  uint32_t tx_power_setting;  /*���书�ʲο���ƽ*/
}radio_public_parm_t;

/*��̨Ƶ�ʷ�Χ*/
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

/*��̨����������flash�У�*/
typedef struct {
	uint32_t flag;                                  /*��ʾ��û�в�����1��ʾ�У���1��ʾû��*/
	//uint32_t tx_freq;                          /*����Ƶ��*/
 	radio_freq_size_t freq_range;        /*Ƶ�ʷ�Χ*/
 	radio_public_parm_t pm;                 /*��������*/
	rx_parm_t rx_parm;
 	ccu_stored_tx_parm_t tx_parm[NUM_FREQS]; /*ȫƵ������3��Ƶ��Ĳ���*/
	char SN[64];
}radio_param_t;

/*��̨��������ǰ��*/
typedef struct {
	  uint32_t power;                      /*���书��*/
	  uint32_t n_w_band;                /*���տ�խ��*/
	  uint32_t tx_freq;                    /*����Ƶ��*/
	  uint32_t rx_freq;                    /*����Ƶ��*/
 	radio_freq_size_t freq_range;   /*Ƶ�ʷ�Χ*/
 	radio_public_parm_t pm;            /*��������*/
	rx_parm_t rx_parm;/*��ǰ���Ͳ���*/
 	ccu_stored_tx_parm_t tx_parm; /*��ǰƵ��Ĳ���*/
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

extern void proc_CFG_send_midlevel(char *buf);    //01�����÷����ź����ĵ�ƽ
extern void proc_CFG_send_range(char *buf);         //02�����÷����źŷ���
extern void proc_CFG_balance_mod1(char *buf);    //03�����õ�̨ƽ��mod1
extern void proc_CFG_balance_mod2(char *buf);    //04�����õ�̨ƽ��mod2
extern void proc_CFG_send_ref_ocsi_param(char *buf);                   //05�����õ�̨���侧�������ƽ
extern void proc_CFG_send_ref_power_param(char *buf);               //06�����õ�̨���书�ʲο���ƽ
extern void proc_CFG_rev_wb_nb_param(char *buf);                         //07�����õ�̨���տ�խ�����ݲ�֧��
extern void proc_CFG_prin_cur_send_freg_param(void);                //08����ӡ��ǰ����Ƶ�ʵ����в���
extern void proc_CFG_prin_all_receive_freg_param(void);              //09����ӡ��ǰ����Ƶ�ʵ����в������ݲ�֧��
extern void proc_CFG_prin_all_param(void);                                        //0A����ӡ���в���
extern void proc_CFG_set_range_freg(char *buf);                                 //0B�����õ�̨Ƶ�ʷ�Χ
extern void proc_CFG_save_current_pub_param(void);                    //0C�����浱ǰ����������Ƶ�ʷ�Χ
extern void proc_CFG_save_current_send_freg_param(void);        //0D�����浱ǰ����Ƶ�ʲ���
extern void proc_CFG_save_receive_freg_param(char *buf);             //0E�����浱ǰ����Ƶ�ʲ������ݲ�֧��
extern void proc_CFG_del_assign_send_freg_param(char *buf);     //0F��ɾ��ָ���ķ���Ƶ�ʲ���
extern void proc_CFG_tx_rx_power(char *buf);                                      //10�������õ�̨����Ƶ�ʣ�����Ƶ�ʺͷ��书��
extern void proc_CFG_pt(void);                                                                 //11�������õ�̨ƽ�����
extern void read_mcu_version(void);                                                       //12����MCU����汾
extern  void proc_CFG_recv_midlevel(char *buf);
extern void proc_CFG_recv_range(char *buf);
#endif // __DEBUG_H__