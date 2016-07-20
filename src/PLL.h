/********************************************************************
版权所有	：Copyright (C) 2013 承联通信
文件名字	：PLL.h
文件功能	：配置PLL寄存器程序头文件
创建日期	：2013-06-30
创建人		：Wuqy
修改标识	：
修改描述	：
*********************************************************************/ 
#ifndef  _PLL_H
#define  _PLL_H

#define SPI_BYTES	3

typedef struct R_LATCH_BITS_
{
	uint32_t ctrl_bits : 2;		//[1:0] control bits
	uint32_t R_cnt : 14;		//[15:2] R counter
	uint32_t ABW : 2;			//[17:16] antibacklash width
	uint32_t test_mode : 2;		//[19:18] test mode
	uint32_t LDP : 1;			//[20] lock detect precision
	uint32_t : 11;				//[31:21] Reserved
}R_LATCH_BITS;

typedef union R_LATCH_
{
	uint32_t all;
	R_LATCH_BITS bit;
}R_LATCH;


typedef struct N_LATCH_BITS_
{
	uint32_t ctrl_bits : 2;		//[1:0] control bits
	uint32_t : 6;				//[7:2] Reserved
	uint32_t N_cnt : 13;		//[20:8] N counter
	uint32_t cp_gain : 1;		//[21] CP gain
	uint32_t : 10;				//[32:22] Reserved
}N_LATCH_BITS;

typedef union N_LATCH_
{
	uint32_t all;
	N_LATCH_BITS bit;
}N_LATCH;


typedef struct FUNC_LATCH_BITS_
{
	uint32_t ctrl_bits : 2;		//[1:0] control bits
	uint32_t cnt_rst : 1;		//[2] counter reset
	uint32_t PD1 : 1;			//[3] powerdown 1
	uint32_t mux_out : 3;		//[6:4] MUXOUT control
	uint32_t PDP : 1;			//[7] phase detector polarity
	uint32_t CP_TS : 1;			//[8] CP threestate
	uint32_t FL_EN : 1;			//[9] fastlock enable
	uint32_t FL_mode : 1;		//[10] fastlock mode
	uint32_t TCC : 4;			//[14:11] timer counter control
	uint32_t CPI1 : 3;			//[17:15] CP current setting 1
	uint32_t CPI2 : 3;			//[20:18] CP current setting 2
	uint32_t PD2 : 1;			//[21] powerdown 2
	uint32_t : 10;				//[32:22] Reserved
}FUNC_LATCH_BITS;

typedef union FUNC_LATCH_
{
	uint32_t all;
	FUNC_LATCH_BITS bit;
}FUNC_LATCH;


extern void config_PLL(void);
extern void proc_CFG_set_pll(char *buf);


#endif
