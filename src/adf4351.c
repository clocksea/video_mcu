#include "adf4351.h"


double cp_map[16]={	0.31,0.63,0.94,1.25,1.56,1.88,2.19,2.50,
					2.81,3.13,3.44,3.75,4.06,4.38,4.69,5.00};



pll_info_t pll_info[2];




unsigned int div_i_f(double num, uint32_t *integer)
{
	unsigned long op;
	unsigned char x;	
	unsigned int i_v,f_v;
	num *=1000;
	op = (unsigned long)num;

	x= op%10;
	op-=x;
	op/=10;
	f_v =x;

	x= op%10;
	op-=x;
	op/=10;	
	f_v +=x*10;

	x= op%10;
	op-=x;
	op/=10;	
	f_v += x*100;	

	x= op%10;
	op-=x;
	op/=10;
	i_v =x;

	x= op%10;
	op-=x;
	op/=10;	
	i_v +=x*10;

	x= op%10;
	op-=x;
	op/=10;	
	i_v += x*100;	 

	x= op%10;
	op-=x;
	op/=10;	
	i_v += x*1000;	

	*integer = i_v;

	return f_v;
}


void delay_dds(int length)
{
	while (length >0)
    	length--;
}

void WriteToADF4351_1(unsigned char count, unsigned char *buf)
{
	unsigned	char	ValueToWrite = 0;
    unsigned	char	i = 0;
	unsigned	char	j = 0;

	IntMasterDisable(); 
	
	delay_dds(200);
	CLR_SCL();
	CLR_LE1();
	delay_dds(200);

	for(i=count;i>0;i--)
 	{
	 	ValueToWrite = *(buf + i - 1);
		for(j=0; j<8; j++)
		{
			if(0x80 == (ValueToWrite & 0x80))
			{
				SET_DATA();	  //Send one to SDO pin
			}
			else
			{
				CLR_DATA();	  //Send zero to SDO pin
			}
			delay_dds(100);
			SET_SCL();
			delay_dds(200);
			ValueToWrite <<= 1;	//Rotate data
			CLR_SCL();
			delay_dds(100);
		}
	}
	CLR_DATA();
	delay_dds(200);
	SET_LE1();
	delay_dds(200);
	CLR_LE1();

	IntMasterEnable();  
}


void adf4351_reset_reg(pll_info_t *pll)
{
	pll->integer = 125;//128;
	pll->fraction = 0;//0;
	pll->mod_value = 16;//2;
	pll->cp_value = 7;//7;
	
	pll->reg0=0x003e8000;//0x00400000;
	pll->reg1=0x08008011;//0x08008011;
	pll->reg2=0x00004047;//0x00004E42;
	pll->reg3=0x000004b3;//0x000004B3;
	pll->reg4=0x00ac803c;//0x00cc803c;
	pll->reg5=0x00580005;//0x00580005;	
}


void adf4351_init(uint32_t num,pll_info_t *pll)
{
	pll->num = num;
	pll->scan_span_hz = 0;
	pll->scan_cur_freq_hz=0;
	pll->scan_start_freq_hz=0;
	pll->scan_stop_freq_hz=0;
	
	adf4351_reset_reg(pll);
	cfg_adf4351(pll);	
}

void cfg_adf4351(pll_info_t *pll)
{
	uint8_t buf[4];

	if(pll->num==1)
	{
		memcpy(buf, &pll->reg5, 4);
		WriteToADF4351_1(4,buf);
		memcpy(buf, &pll->reg4, 4);
		WriteToADF4351_1(4,buf);
		memcpy(buf, &pll->reg3, 4);
		WriteToADF4351_1(4,buf);
		memcpy(buf, &pll->reg2, 4);
		WriteToADF4351_1(4,buf);
		memcpy(buf, &pll->reg1, 4);
		WriteToADF4351_1(4,buf);
		memcpy(buf, &pll->reg0, 4);
		WriteToADF4351_1(4,buf);		
	}		
}

void rf_out(pll_info_t *pll)
{
	unsigned int mod_v[10]={5,25,50,125,250,500,1000,2000,4000};
	unsigned char div,flag;
	unsigned int frac_value;
	unsigned int mod_value;
	double n_value,op,y;
	double ref_clk=19.2;
	unsigned long temp;
	unsigned int i,x;
	double rf_fre_value_mhz;
	uint8_t buf[4];
	unsigned int fraction, integer;

	rf_fre_value_mhz = pll->rf_freq_hz/(1000*1000);
		
	if(rf_fre_value_mhz<69)
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00E00000;
		div=64;
	}
	else if(rf_fre_value_mhz<138)
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00D00000;
		div=32;
	}
	else if(rf_fre_value_mhz<275)
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00C00000;
		div=16;
	}
	else if(rf_fre_value_mhz<550)
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00B00000;
		div=8;
	}
	else if(rf_fre_value_mhz<1100)
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00A00000;
		div=4;
	}
	else if(rf_fre_value_mhz<2200)
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00900000;
		div=2;
	}
	else
	{
		pll->reg4 &= 0x000FFFFF;
		pll->reg4 |= 0x00800000;
		div=1;
	}
		
	op = ref_clk/div;
	n_value = rf_fre_value_mhz/op;
	fraction=div_i_f(n_value,&integer);
	temp = integer;
	temp <<=15;
	pll->reg0 &= 0x00007FFF;
	pll->reg0 |= temp;
	
	if(fraction==0)
	{
		frac_value=0;
		mod_value=2;
	}
	else
	{
		flag=1;
		y=fraction;
		y /= 1000;
		while(flag)
		{
			for(i=0;i<9;i++)
			{
				op=mod_v[i]*y;
				x = div_i_f(op,&integer);
				if(integer<mod_v[i])
				{
					if(x==0)
					{
						frac_value=integer;	
						mod_value=mod_v[i];
						i=10;
						flag=0;
					}
				}
			}				
		}
	}
		
	frac_value<<=3;
	pll->reg0 &= 0xEfff8007;
	pll->reg0 |= frac_value;
	mod_value <<=3;
	pll->reg1 &= 0x1FFFE007;
	pll->reg1 |= mod_value;

	pll->reg2 &= 0xFFFFE1FF;
	pll->reg2 |= pll->cp_value;

	pll->div = div;
	pll->fraction = fraction;
	pll->frac_value = frac_value;
	pll->integer = integer;
	pll->mod_value = mod_value;

	cfg_adf4351(pll);
}


void printf_pll_info(pll_info_t *pll)
{
	myprintf("\n\r--------pll-%d参数如下--------\n\r",pll->num);
	
	myprintf("rf_freq_hz:%dHz\n\r",pll->rf_freq_hz);
	myprintf("reg0:0x%08x\n\r",pll->reg0);
	myprintf("reg1:0x%08x\n\r",pll->reg1);
	myprintf("reg2:0x%08x\n\r",pll->reg2);
	myprintf("reg3:0x%08x\n\r",pll->reg3);
	myprintf("reg4:0x%08x\n\r",pll->reg4);
	myprintf("reg5:0x%08x\n\r",pll->reg5);

	myprintf("integer:%d\n\r",pll->integer);	
	
	myprintf("fraction:%d\n\r",pll->fraction);	
	myprintf("frac_value:%d\n\r",pll->frac_value);	
	myprintf("mod_value:%d\n\r",pll->mod_value);	
	myprintf("div:%d\n\r",pll->div);	
	myprintf("cp_value:%d\n\r",pll->cp_value);	
}



