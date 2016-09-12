
#ifndef  _INCLUDES_H
#define  _INCLUDES_H

#include <stdio.h>                      //C标准库头文件
#include <string.h>


#include "hw_types.h"
#include "hw_ints.h"
#include "hw_memmap.h"
#include "hw_gpio.h"
#include "debug.h"
#include "gpio.h"
#include "sysctl.h"
#include "uart.h"
#include "flash.h"
#include "interrupt.h"
#include "I2c.h"
#include "Timer.h"
#include "radio.h"
#include "PLL.h"
#include "i2s.h"
#include "adc.h"
#include "ssi.h"

#include "my_uart.h"
#include "adf4351.h"

#define  FLASH_BASE_ADDR (0x1FC00)

#define  LED_PERIPH             SYSCTL_PERIPH_GPIOA
#define  LED_PORT                GPIO_PORTA_BASE
#define  LED_PIN                  GPIO_PIN_6

#define  PLLLE_1_PERIPH             SYSCTL_PERIPH_GPIOH
#define  PLLLE_1_PORT                GPIO_PORTH_BASE
#define  PLLLE_1_PIN                  GPIO_PIN_1

#define  PLLLE_2_PERIPH             SYSCTL_PERIPH_GPIOC
#define  PLLLE_2_PORT                GPIO_PORTC_BASE
#define  PLLLE_2_PIN                  GPIO_PIN_6


#define  PLLCLK_PERIPH             SYSCTL_PERIPH_GPIOD
#define  PLLCLK_PORT                GPIO_PORTD_BASE
#define  PLLCLK_PIN                  GPIO_PIN_6

#define  PLLDATA_PERIPH             SYSCTL_PERIPH_GPIOF
#define  PLLDATA_PORT                GPIO_PORTF_BASE
#define  PLLDATA_PIN                  GPIO_PIN_0

#define  PLLCE_1_PERIPH             SYSCTL_PERIPH_GPIOH
#define  PLLCE_1_PORT                GPIO_PORTH_BASE
#define  PLLCE_1_PIN                  GPIO_PIN_0

#define  PLLCE_2_PERIPH             SYSCTL_PERIPH_GPIOC
#define  PLLCE_2_PORT                GPIO_PORTC_BASE
#define  PLLCE_2_PIN                  GPIO_PIN_7

#define  PLLPDBRF_1_PERIPH             SYSCTL_PERIPH_GPIOB
#define  PLLPDBRF_1_PORT                GPIO_PORTB_BASE
#define  PLLPDBRF_1_PIN                  GPIO_PIN_6

#define  RF_CTRL_1_PERIPH             SYSCTL_PERIPH_GPIOB
#define  RF_CTRL_1_PORT                GPIO_PORTB_BASE
#define  RF_CTRL_1_PIN                  GPIO_PIN_4

#define  RF_CTRL_2_PERIPH             SYSCTL_PERIPH_GPIOB
#define  RF_CTRL_2_PORT                GPIO_PORTB_BASE
#define  RF_CTRL_2_PIN                  GPIO_PIN_3




#define SET_LE1()		GPIOPinWrite(PLLLE_1_PORT, PLLLE_1_PIN, PLLLE_1_PIN)
#define CLR_LE1()		GPIOPinWrite(PLLLE_1_PORT, PLLLE_1_PIN, 0)
	
#define	SET_SCL()		GPIOPinWrite(PLLCLK_PORT, PLLCLK_PIN, PLLCLK_PIN)
#define	CLR_SCL()		GPIOPinWrite(PLLCLK_PORT, PLLCLK_PIN, 0)
	
#define SET_DATA()		GPIOPinWrite(PLLDATA_PORT, PLLDATA_PIN, PLLDATA_PIN)
#define CLR_DATA()		GPIOPinWrite(PLLDATA_PORT, PLLDATA_PIN, 0)
	
#define SET_CE2()		GPIOPinWrite(PLLCE_2_PORT, PLLCE_2_PIN, PLLCE_2_PIN)	
#define CLR_CE2()		GPIOPinWrite(PLLCE_2_PORT, PLLCE_2_PIN, 0)

#define SET_RF_CTRL_1()	GPIOPinWrite(RF_CTRL_1_PORT, RF_CTRL_1_PIN, RF_CTRL_1_PIN)	
#define CLR_RF_CTRL_1()	GPIOPinWrite(RF_CTRL_1_PORT, RF_CTRL_1_PIN, 0)

#define SET_RF_CTRL_2()	GPIOPinWrite(RF_CTRL_2_PORT, RF_CTRL_2_PIN, RF_CTRL_2_PIN)	
#define CLR_RF_CTRL_2()	GPIOPinWrite(RF_CTRL_2_PORT, RF_CTRL_2_PIN, 0)


#define MID_FREQ_HZ		(480*1000*1000)/*480MHz*/

#define MAX_FREQ_HZ		(2600*1000*1000)/*2600MHz*/
#define MIN_FREQ_HZ		(800*1000*1000)/*800MHz*/
#define FREQ_1_MHZ		(1000*1000)/*1MHZ*/
#define FREQ_1_KHZ		(1000)/*1KHZ*/


#define SCAN_STATUS_IDLE (0)
#define SCAN_STATUS_WAITTING_PLLLOCK (1)
#define SCAN_STATUS_READING_RSSI (2)

#define ADC_STAUTS_IDLE (0)
#define ADC_STAUTS_SAMPLING (1)
#define ADC_STAUTS_SAMPLE_DONE (2)




/*------------------------软件版本信息记录--------------*/
/*
V1.0.0 add by zhongh 2016-7-30
	a.初始版本
V1.1.0 add by zhongh 2016-9-12
	a.扫描基本OK
*/
#define VER_INFO "V1.1.0"



extern radio_param_t radio_param;
extern void init_software(void);                       //定义于ccu_uart.c文件中
extern void proc_ccu_uart_buf(void);
extern void timer1Init(void);

extern unsigned CCH_flag;
extern int heart_flag;
extern int alarm_flag;
//extern int32_t handle_heart(char *buf, unsigned short len, int32_t *alarm, int32_t *tx_freq, int32_t *rx_freq, int32_t *tx_pwr, unsigned char *band);

extern void UART_CCU_rx_isr(void);	//CCU/PC UART接收中断服务程序，定义于ccu_uart.c文件中

extern unsigned int board_ver;
extern unsigned long uart_for_ccu;
extern unsigned long uart_for_920;

#endif
