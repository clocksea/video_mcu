
#ifndef  _INCLUDES_H
#define  _INCLUDES_H

#include <stdio.h>                      //C标准库头文件
#include <string.h>


#include "inc/hw_types.h"
#include "inc/hw_ints.h"
#include "inc/hw_memmap.h"
#include "inc/hw_gpio.h"
#include "driverlib/debug.h"
#include "driverlib/gpio.h"
#include "driverlib/sysctl.h"
#include "driverlib/uart.h"
#include "driverlib/flash.h"
#include "driverlib/interrupt.h"
#include "driverlib/I2c.h"
#include "utils/uartstdio.h"
#include "driverlib/Timer.h"
#include "radio.h"
#include "920_uart.h"
#include "PLL.h"

#define  FLASH_BASE_ADDR (0x1FC00)

extern radio_param_t radio_param;
extern void init_software(void);                       //定义于ccu_uart.c文件中
extern void init_ccu_uart(void);
extern void proc_ccu_uart_buf(void);
extern void timer1Init(void);
extern int32_t recv_radio_hbreq(radio_uart_t *data, int32_t *alarm, int32_t *tx_freq, int32_t *rx_freq, int32_t *tx_pwr, unsigned char *band);	//调试
extern unsigned int c_timer;
extern unsigned int cch_count;
extern unsigned CCH_flag;
extern int heart_flag;
extern int alarm_flag;
//extern int32_t handle_heart(char *buf, unsigned short len, int32_t *alarm, int32_t *tx_freq, int32_t *rx_freq, int32_t *tx_pwr, unsigned char *band);

extern void UART_CCU_rx_isr(void);	//CCU/PC UART接收中断服务程序，定义于ccu_uart.c文件中

extern unsigned int board_ver;
extern unsigned long uart_for_ccu;
extern unsigned long uart_for_920;

#endif
