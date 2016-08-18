/******************************************************************************

                  ��Ȩ���� (C), 2001-2021, ****

 ******************************************************************************
  �� �� ��   : cmd_proc.h
  �� �� ��   : ����
  ��    ��   : zhonghai
  ��������   : 2016��8��2��
  ����޸�   :
  ��������   : cmd_proc.c ��ͷ�ļ�
  �����б�   :
  �޸���ʷ   :
  1.��    ��   : 2016��8��2��
    ��    ��   : zhonghai
    �޸�����   : �����ļ�

******************************************************************************/

/*----------------------------------------------*
 * ����ͷ�ļ�                                   *
 *----------------------------------------------*/
#include "includes.h"

/*----------------------------------------------*
 * �ⲿ����˵��                                 *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ⲿ����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �ڲ�����ԭ��˵��                             *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ȫ�ֱ���                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ģ�鼶����                                   *
 *----------------------------------------------*/

/*----------------------------------------------*
 * ��������                                     *
 *----------------------------------------------*/

/*----------------------------------------------*
 * �궨��                                       *
 *----------------------------------------------*/

#ifndef __CMD_PROC_H__
#define __CMD_PROC_H__


#ifdef __cplusplus
#if __cplusplus
extern "C"{
#endif
#endif /* __cplusplus */

/******************************************************************************
 *	 ��Ϣ����·�ɱ�
**/	
typedef void (*pmsgfunc)(char *, unsigned char); /* �����ص�����ǩ�� */

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
