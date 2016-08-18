/******************************************************************************

                  版权所有 (C), 2001-2021, ****

 ******************************************************************************
  文 件 名   : cmd_proc.h
  版 本 号   : 初稿
  作    者   : zhonghai
  生成日期   : 2016年8月2日
  最近修改   :
  功能描述   : cmd_proc.c 的头文件
  函数列表   :
  修改历史   :
  1.日    期   : 2016年8月2日
    作    者   : zhonghai
    修改内容   : 创建文件

******************************************************************************/

/*----------------------------------------------*
 * 包含头文件                                   *
 *----------------------------------------------*/
#include "includes.h"

/*----------------------------------------------*
 * 外部变量说明                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 外部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 内部函数原型说明                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 全局变量                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 模块级变量                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 常量定义                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * 宏定义                                       *
 *----------------------------------------------*/

#ifndef __CMD_PROC_H__
#define __CMD_PROC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/******************************************************************************
 *	 消息处理路由表
**/	
typedef void (*pmsgfunc)(char *, unsigned char); /* 信令处理回调函数签名 */

typedef struct{
	char *cmd;
	char *info;
	pmsgfunc func;
}at_func_map_t;


extern void init_software(void);
extern void __pll_reg0_set(uint32_t ch, uint32_t reg, uint32_t int_value, uint32_t frac);
extern void __pll_reg1_set(uint32_t ch, uint32_t reg, uint32_t adj, uint32_t prescaler, uint32_t phase, uint32_t mod);
extern void __pll_reg2_set(uint32_t ch, uint32_t reg, uint32_t noise_mode, uint32_t muxout, uint32_t reference_double, uint32_t reference_div_by_2,
         						uint32_t r_count, uint32_t double_buffer, uint32_t charge_pump, uint32_t ldf, uint32_t ldp, uint32_t pd, uint32_t power_down, 
         						uint32_t cp, uint32_t counter_reset);
extern void __pll_reg3_set(uint32_t ch, uint32_t reg, uint32_t band_select, uint32_t abp, uint32_t charge_cancelation, uint32_t csr, uint32_t clk_div_mode, 
         						uint32_t clk_div_value);
extern void __pll_reg4_set(uint32_t ch, uint32_t reg, uint32_t feed_back_select, uint32_t rf_div_select, uint32_t band_select_clk_div, uint32_t vco_power_down,
         						uint32_t mtld, uint32_t aux_output_select, uint32_t aux_out, uint32_t aux_output_power, uint32_t rf_out, uint32_t output_power);
extern void __pll_reg5_set(uint32_t ch, uint32_t reg, uint32_t ld_pin_mode);
extern void proc_AT_cmd(char *buf, unsigned char len);
extern void proc_INFO_cmd(char *buf, unsigned char len);
extern void proc_PLLGET_cmd(char *buf, unsigned char len);
extern void proc_PLLSET_cmd(char *buf, unsigned char len);
extern void proc_SMFREQ_cmd(char *buf, unsigned char len);
extern void proc_SSFREQ_cmd(char *buf, unsigned char len);
extern void proc_uart_buf(uart_info_t *uart);
extern void proc_uart_cmd(char *buf, unsigned char len);
extern void proc_SMPLLCP_cmd(char *buf, unsigned char len);
extern void proc_RFCTRL_cmd(char *buf, unsigned char len);
extern void proc_PLLINFO_cmd(char *buf, unsigned char len);
extern void myhelp(void);
#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif /* __cplusplus */


#endif /* __CMD_PROC_H__ */
