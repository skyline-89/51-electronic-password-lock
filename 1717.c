#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char  
#define uint unsigned int 
#define OP_WRITE 0xa0          // ������ַ�Լ�д����� 
#define OP_READ  0xa1           // ������ַ�Լ���ȡ���� 
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();}; 
//************************����˿�*************************
sbit SDA = P2^0; //������
sbit SCL = P2^1; //ʱ����
sbit dula=P2^6;
sbit wela=P2^7;
sbit alarm =P2^3;
sbit LCD_HH=P3^1;	
sbit LCD_EN=P3^4;	
sbit LCD_RS=P3^5;
sbit LCD_RW=P3^6;
sbit LCD_RD = P3^7;
//************************�������鼰ȫ�ֱ���*************************
uchar v,w,mark3,free;
uchar judge;
uchar code initial_pass[6]={6,1,2,3,4,5};	//��ʼ����
//***************�洢1602��ʾ������*******************************
uchar code mun_to_char[] = {"0123456789ABCDEF"}; /*�������ָ�ASCII��Ĺ�ϵ*/
uchar code dis1[]={"INPUT PASSWORD"};
uchar code dis2[]={"OPEN"};
uchar code dis3[]={"ERROR"};
uchar code dis4[]={"NEW PASSWORD"};
uchar code dis5[]={"OPEN  OR  CHANGE"};
uchar code dis6[]={"SUCCESSFUL"};
uchar code dis7[]={"THE PASSWORD IS"};
uchar code dis8[]={"DIFFERENT!"};
uchar code dis9[]={"INPUT AGAIN"};
uchar code dis10[]={"About To Lock..."};
uchar code dis11[]={"TEMPERATURE:"};
uchar data dis_tem[]={"00.0"};
//*****************��������24C02�ĺ���*****************
void delayms(uint ms);
void start(); 
void stop(); 
void change_passwords();
bit match_passwords();
void write_byte(uchar addr, uchar write_data);
uchar read_random(uchar random_addr); 
uchar data_in(); 
bit data_out(uchar write_data); 
//*****************************************����
uchar passwords[8];//��ʱ�洢����
bit aleady_confirm;//�Ƿ���ȷ��
uchar pw_length;//�û��Ѿ���������볤��
uchar temp,num;//���ھ��󰴼�ʶ��
void manager();//����Ա����
void process1();//��������
void process2();//ѡ��
void over_op();//ȷ�ϰ�����Ĳ���
void over_op2();//������ȷ��Ĳ���
void delay(uchar);//�ӳٺ���
//**************************����ɨ��
void scanKeyboard0();
void scanKeyboard1();
void scanKeyboard2();
//*******************1602********************************
bit lcd_busy();
void write_1602com(uchar com);
void write_1602dat(uchar dat);
void lcd_init(void);
void display_shuru(void);
void display_error();
void display_choose();
void display_successful();
void display_again();
void display_different();
void display_change();
void display_lock();
void exter0_init();
void xuanze1();
void xuanze2();
void xianshi();
void tuige();
//******************����********************************** 
uchar chaoshi_led();
void open_led(void);
void shanshuo_led();
void fault_led();
void chaoshi_jingbao();
//*******************�ӳٺ���****************************
void delay1ms(void);
void delay5ms(void);
void delay50ms(void);
void delay500ms(void);
void delay1s500ms(void);
void delay2s500ms(void);
void delay5s();
void delay10s(void);
/*******************18B20***************************************/
sbit DQ =P2^2;  //����DS18B20ͨ�Ŷ˿�
void Init_DS18B20();
uchar ReadOneChar();
void WriteOneChar(unsigned char dat);
uint ReadTemperature();
void display_temperature();
void delay2(unsigned int i);
//***********************������***************************
void main() 
{ 
	uchar  i;
	uchar attempts;
	bit mark;
	free=1;
	judge=read_random(10);
	if(!judge)
	{
	write_byte(10,1);
	for(i=0;i<7;i++)
	{
		write_byte(i,initial_pass[i]);
	} 
}
	//����һ����������
	loop1:
	display_temperature();
	mark=0;
	loop2:
	display_shuru();
	attempts=0;
	do{
		//���ο���ʧ�ܲ���
		if(attempts==3)
		{
			uchar k;	  
			display_lock();
		 	 shanshuo_led();
			for(k=0;k<2;k++)
			{
				write_1602com(0x08);
				delay500ms();
				write_1602com(0x0c);
				delay500ms();
			}			
			write_1602com(0x01);
			delay50ms();
			manager();
			free=1;
			goto loop2;
			break;
		}
		//�������Ĳ���
		else if(attempts!=0)
		{
		display_error();
		delay50ms();
		fault_led();
		write_1602com(0x01);
		display_shuru();
		}
        process1();

		while(mark3)
			goto loop2;
		mark=match_passwords();	
		attempts+=1;  
	}
	while(!mark); 
	display_choose();
	process2();
	free=1;
	goto loop2;
	
} 
//*********************************************************
void manager()
{
	uchar manage;
	uchar read34;
	manage=0;
	LCD_RD=1;
	
	while(!manage)
	{
		read34=LCD_RD;
		if(!read34)
		{
			delay1s500ms();
			open_led();
			do{read34=LCD_RD;}while(!read34);
			manage=1;
		}
	}
}
//**********************************************************
void change_passwords()
{
	uchar j;
	uchar mark2;
	mark2=0;
	do{
		//��������
		process1();
		write_byte(0,pw_length);
		for(j=1;j<=pw_length;j++)
			write_byte(j,passwords[j-1]);
		//ƥ������
		display_again();
		delay500ms();
		process1();
		mark2=match_passwords();
		write_1602com(0x01);
		if(!mark2)
		{
			delay50ms();
			display_different();  
			fault_led();
			delay500ms();
			write_1602com(0x01);
			display_change();
		}
	}while(!mark2);
	display_successful();
	delay1s500ms();
	//�޸ĳɹ���������������
}
//**********************************************************
bit match_passwords()
//����ƥ��
{
	uchar i;
	uchar pos=read_random(0);
	if(pos==pw_length)
	{
		uchar read_data;
		for(i=1;i<=pw_length;i++)
		{
			read_data=read_random(i);
			if(passwords[i-1]!=read_data)
				break;
			if(i==pw_length)
				return 1;
		}
	}
	return 0;
}
//**********************************************************
void write_byte(uchar addr, uchar write_data) 
// ��ָ����ַaddr��д������write_data 
{ 
	SDA = 1; 
	SCL = 1;
	start(); 
	data_out(OP_WRITE); 
	data_out(addr); 
	data_out(write_data); 
	stop(); 
	delayms(10);          // д������ 
} 


uchar read_random(uchar random_addr) 
// ��ָ����ַ��ȡ�洢���ֽ�
{ 
	uchar readda;
	SDA = 1;
	SCL = 1;
	start();
	data_out(OP_WRITE);
	data_out(random_addr);
	start();
	data_out(OP_READ);
	readda=data_in();
	stop();
	return readda;
} 

uchar data_in() 
// ��AT24C02�Ƴ����ݵ�MCU 
{ 
	uchar i,read_data; 
	for(i = 0; i < 8; i++) 
	{ 
   	 	SCL = 1; 
   		 read_data <<= 1; 
    		read_data |= SDA; 
   		 SCL = 0; 
	} 
	return(read_data); 
} 

bit data_out(uchar write_data)  
// ��MCU�Ƴ����ݵ�AT24C02 
{ 
	uchar i; 
	bit ack_bit; 
	for(i = 0; i < 8; i++)   // ѭ������8��λ 
	{ 
		SDA = (bit)(write_data & 0x80); 
		_nop_(); 
		SCL = 1; 
		delayNOP(); 
		SCL = 0; 
		write_data <<= 1; 
	} 
	SDA = 1;                     // ��ȡӦ�� 
	delayNOP(); 
	SCL = 1; 
	delayNOP(); 
	ack_bit = SDA; 
	SCL = 0; 
	return ack_bit;            // ����AT24C02Ӧ��λ 
} 
 
void start() 
 //��ʼλ 
{ 
	SDA = 1; 
	SCL = 1; 
	delayNOP(); 
	SDA = 0; 
	delayNOP(); 
	SCL = 0; 
} 

void stop() 
 // ֹͣλ 
{ 
	SDA = 0; 
	delayNOP(); 
	SCL = 1; 
	delayNOP(); 
	SDA = 1; 
} 


//**********************************************************
void process1()
//�����������
{
	pw_length=0;
	aleady_confirm=0;
	exter0_init();
	v=1;
	w=0;
	while(!aleady_confirm)
	{	
		mark3=chaoshi_led();
		if(mark3)
		{
		chaoshi_jingbao();
		break;
		}
		scanKeyboard1();
	}
	v=0;
	TR0=0;
}
//***************************
void process2()
//ѡ���������޸�����
{
	aleady_confirm=0;
	while(!aleady_confirm)
	{
		scanKeyboard2();
	}
	over_op2();
}
//****************************
void over_op()
{
	if(num==10)//ִ���˸����
	{
		if(pw_length==0)
		;
		else
		{
			w=0;
			tuige();
			pw_length--;
			passwords[pw_length]=10;
		}
	}
	else if(num==11||pw_length==8)//ִ��ȷ�ϲ���
	{
		aleady_confirm=1;
	}

	else //����������ֱ���
	{
		TR0=1;
		w=0;
		xianshi();
		passwords[pw_length]=num;
		pw_length++;											   
	}
}
//*****************************
void over_op2()
{
	if(num==12)//ѡ��һ����
	{
		xuanze1();
		delay50ms();
		open_led();
	}
	if(num==13)//ѡ����޸�����
	{
		xuanze2();
		change_passwords();
	}
}
//*****************************
void scanKeyboard1(){
	P3=0xfe;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			switch(temp)
			 {					
				case 0xe0: num=0;
					break;
				case 0xd0: num=1;
					break;
				case 0xb0: num=2;
					break;
				case 0x70: num=3;
					break;
			}
			do
			{
			temp=P3;
			temp=temp&0xf0;
			}while(temp!=0xf0);
			over_op();
		 }
	 }

	P3=0xfd;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			switch(temp)
			{
				case 0xe0: num=4;
					break;
				case 0xd0: num=5;
					break;
				case 0xb0: num=6;
					break;
				case 0x70: num=7;
					break;
			}
		    do
			{
			temp=P3;
			temp=temp&0xf0;
			}while(temp!=0xf0);
			over_op();
		}
	}

	P3=0xfb;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			switch(temp)
			{
				case 0xe0: num=8;
					break;
				case 0xd0: num=9;
					break;
				case 0xb0: num=10;
					break;
				case 0x70: num=11;
					break;
			}
			do
			{
			temp=P3;
			temp=temp&0xf0;
			}while(temp!=0xf0);
			over_op();
		}
	}
}
//*************************
void scanKeyboard2()
{
	P3=0xf7;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			switch(temp)
			{
				case 0xe0: num=12;
					break;
				case 0xd0: num=13;
					break;
			}
			P3=0x00;
			aleady_confirm=1;
		}
	}
}
//************************************
void scanKeyboard0(){
	P3=0xfe;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			free=0;
			}
			do
			{
			temp=P3;
			temp=temp&0xf0;
			}while(temp!=0xf0);
	}

	P3=0xfd;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			free=0;
			}
			do
			{
			temp=P3;
			temp=temp&0xf0;
			}while(temp!=0xf0);
		}

	P3=0xfb;
	temp=P3;
	temp=temp&0xf0;
	if(temp!=0xf0)
	 {
		delay50ms();
		if(temp!=0xf0)
		 {
			free=0;
			}
			do
			{
			temp=P3;
			temp=temp&0xf0;
			}while(temp!=0xf0);
		}
}
//------------------1602-------------------------
//-------------------��æ---------------
bit lcd_busy() 
{ 
bit result; 
LCD_RS = 0; 
LCD_RW = 1; 
LCD_EN = 1;
_nop_(); 
_nop_();
_nop_();
_nop_(); 
result = (bit)(P0&0x80); 
LCD_EN = 0; 
return result; 
}
//--------------дָ��--------------
void write_1602com(uchar com)
{
while(lcd_busy());
LCD_RS = 0; 
LCD_RW = 0; 
LCD_EN = 0; 
_nop_(); 
_nop_(); 
P0 = com; 
_nop_(); 
_nop_();
_nop_();
_nop_(); 
LCD_EN = 1; 
_nop_(); 
_nop_(); 
_nop_();
_nop_();
LCD_EN = 0;
}
//--------------д����-------------
void write_1602dat(uchar dat)
{
while(lcd_busy()); 
LCD_RS = 1; 
LCD_RW = 0;
LCD_EN = 0;
P0 = dat; 
_nop_(); 
_nop_(); 
_nop_();
_nop_();
LCD_EN = 1;
_nop_();
_nop_();
_nop_();
_nop_();
LCD_EN = 0; 
}
//---------------��ʼ��----------------------
void lcd_init(void)
{
delay5ms();
delay5ms();
delay5ms();
write_1602com(0x38); //16*2��ʾ��5*7����8λ���� �����ù���ģʽ
delay5ms();
delay5ms(); 
write_1602com(0x38); 
delay5ms();
delay5ms();
write_1602com(0x38); 
delay5ms(); 
write_1602com(0x0c); //��ʾ�����ع�� 
delay5ms(); 
write_1602com(0x06); //�����ƶ���� ���������ƶ�
delay5ms(); 
write_1602com(0x01); //���LCD����ʾ���� 
delay5ms(); 
}
//----------------��ʾ����������ʾ--------------
void display_shuru(void)
{
uchar i;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x80);		//��ʾλ��Ϊ��һ�е�һ���ַ�
i=0;
while(dis1[i]!='\0')
{
write_1602dat(dis1[i]);	  //��ʾ��INPUT PASSWORD��
i++;
}
}										                             
//************��ʾ��ERROR��***********
void display_error()
{
uchar i;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x80);		//��ʾλ��Ϊ��һ�е�һ���ַ�
i=0;
while(dis3[i]!='\0')
{
write_1602dat(dis3[i]);	  //��ʾ��ERROR��
i++;
}				                                          
}


//****************��ʾ��SUCCESSFUL��***
void display_successful()
{
uchar i;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x82);	//��ʾλ��Ϊ��һ�е������ַ�
i=0;
while(dis6[i]!='\0')
{
write_1602dat(dis6[i]);	 
i++;
}				                                          
}
//***********��ʾ�ٴ�����************
void display_again()
{
uchar i;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x80);		//��ʾλ��Ϊ��һ�е�1���ַ�
i=0;
while(dis9[i]!='\0')
{
write_1602dat(dis9[i]);	 
i++;
}				                                          
}
//**************��ʾ���벻ͬ����***********
void display_different()
{
uchar i,k;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x80);
i=0;
while(dis7[i]!='\0')
{
write_1602dat(dis7[i]);	 
i++;
}
delay50ms();
write_1602com(0x0c3);
k=0;
while(dis8[k]!='\0')
{
write_1602dat(dis8[k]);	 
k++;
}				                                          
}				                                          


//*******************ѡ�����**********
void display_choose()
{
uchar i;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x80);
i=0;
while(dis5[i]!='\0')
{
write_1602dat(dis5[i]);
i++;
}
}
//**************ѡ��ڶ��е����(�����*******
void xuanze1()
{
uchar i=0,k;
write_1602com(0x0c2);
write_1602dat('^');
delay500ms();
for(k=0;k<2;k++)
{
write_1602com(0x08);
delay500ms();
write_1602com(0x0c);
delay500ms();
}
write_1602com(0x01);
delay50ms();
write_1602com(0x86);
write_1602com(0x06);
while(dis2[i]!='\0')
{
write_1602dat(dis2[i]);
i++;
}
}
//********�ڶ����Ҽ�**********
void xuanze2()
{
uchar i=0,k;
write_1602com(0x0cd);
write_1602dat('^');
delay500ms();
for(k=0;k<2;k++)
{
write_1602com(0x08);
delay500ms();
write_1602com(0x0c);
delay500ms();
}
write_1602com(0x01);
delay50ms();
display_change();
}

//***********���ܽ���************
void display_change()
{
uchar i;
i=0;
write_1602com(0x80);
write_1602com(0x06);
while(dis4[i]!='\0')
{
write_1602dat(dis4[i]);
i++;
}
}
//**********������ʾ***********
void display_lock()
{
uchar i;
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
lcd_init();
write_1602com(0x80);	//��ʾλ��Ϊ��һ�е�1���ַ�
i=0;
while(dis10[i]!='\0')
{
write_1602dat(dis10[i]);	 
i++;
}				                                          
}
//---------------------------��������ʾ�� * ��

void xianshi()
{
P0=0X00;
dula=1;
wela=0;
delay1ms();
dula=0;
wela=0;
delay1ms();
delay1ms();
delay1ms();
write_1602com(0x0c0+pw_length);
delay5ms();
write_1602com(0x0c0+pw_length);
delay5ms();
write_1602com(0x14);
write_1602dat('*');
}
//�˸�
void tuige()
{
	P0=0X00;
    dula=1;
    wela=0;
    delay1ms();
    dula=0;
    wela=0;
    delay1ms();
    delay1ms();
    delay1ms();	  	
	write_1602com(0x10);
write_1602com(0x0c0+pw_length);
	write_1602dat(' ');
}



//******************�����͵ƹ�************
//�����ƣ�����������һ����
void open_led()
{
uchar temp;
temp=0xfe;
P1=temp;
delay50ms();
temp=0xfc;
P1=temp;
delay50ms();
temp=0xf8;
P1=temp;
delay50ms();
temp=0xe0;
P1=temp;
alarm=0;
delay50ms();
temp=0xc0;
P1=temp;
alarm=1;
delay50ms();
temp=0x80;
P1=temp;
delay50ms();
temp=0x00;
P1=temp;
delay50ms();
temp=0xff;
P1=temp;
delay50ms();
temp=0x00;
P1=temp;
delay50ms();
temp=0xff;
P1=temp;
alarm=0;
delay50ms();
temp=0x00;
P1=temp;
alarm=1;
delay5s();
temp=0xff;
P1=temp;
}
//�����ξ�������˸;
void shanshuo_led()
{
uchar temp1,i;
alarm=0;
for(i=0;i<40;i++)
{
temp1=0x00;
P1=temp1;
delay50ms();
temp1=0xff;
P1=temp1;
delay50ms();
}
delay50ms();
alarm=1;
}
//�������,������
void fault_led()
{
uchar temp3,i;
alarm=0;
for(i=0;i<5;i++)
{
temp3=0x0f;
P1=temp3;
delay50ms();
temp3=0xff;
P1=temp3;
delay50ms();
}
temp3=0x0f;
P1=temp3;
delay500ms();
delay1s500ms();
temp3=0xff;
P1=temp3;
alarm=1;
}
//**********���е���ʱ�ӳ���*************
void delayms(uint ms)  
// 1ms��ʱ�ӳ��� 
{ 
   uchar k; 
   while(ms--) 
   { 
      for(k = 0; k < 120; k++); 
   } 
}
void delay(uchar x)
{
	uchar a,b;
	for(a=x;a>0;a--)
	 for(b=200;b>0;b--);
}
//��ʱ1ms;
void delay1ms(void)  
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=142;b>0;b--)
            for(a=2;a>0;a--);
}
//��ʱ5ms
void delay5ms(void)  
{
    unsigned char a,b;
    for(b=19;b>0;b--)
        for(a=130;a>0;a--);
}
//��ʱ50ms;
void delay50ms(void)   
{
    unsigned char a,b;
    for(b=173;b>0;b--)
        for(a=143;a>0;a--);
}
//��ʱ500ms
void delay500ms(void)   
{
    unsigned char a,b,c;
    for(c=23;c>0;c--)
        for(b=152;b>0;b--)
            for(a=70;a>0;a--);
}
//��ʱ1.5s;
void delay1s500ms(void)   
{
    unsigned char a,b,c;
    for(c=127;c>0;c--)
        for(b=96;b>0;b--)
            for(a=60;a>0;a--);
}
//��ʱ2.5s
void delay2s500ms(void)   
{
    unsigned char a,b,c,n;
    for(c=229;c>0;c--)
        for(b=214;b>0;b--)
            for(a=24;a>0;a--);
    for(n=1;n>0;n--);
    _nop_();  
}
//��ʱ10s;
void delay10s(void) 
{
    unsigned char a,b,c;
    for(c=191;c>0;c--)
        for(b=189;b>0;b--)
            for(a=137;a>0;a--);
    _nop_();   
}
//��ʱ5s;
void delay5s()   
{
    unsigned char a,b,c;
    for(c=165;c>0;c--)
        for(b=100;b>0;b--)
            for(a=150;a>0;a--);
    _nop_(); 
    _nop_(); 
}
//************************������ʱ
uchar chaoshi_led()
{
if(w==60)
{
w=0;
v=0;
return 1;
}
return 0;
}
void exter0_init()
{
TMOD=0x01;
TH0=(65536-50000)/256;
TL0=(65536-50000)%256;
EA=1;
ET0=1;
//TR0=1;
}
void exter0()interrupt 1
{
TH0=(65536-50000)/256;
TL0=(65536-50000)%256;
if(v==1)
{
w++;
}
}
void chaoshi_jingbao()
{
P1=0;
alarm=0;
delay1s500ms();
alarm=1;
P1=1;
}
/***************************18B20******************************/
//DS18B20��ʱ����
void delay2(unsigned int i)
{
	while(i--);
}
//DS18B20��ʼ������
void Init_DS18B20(void)
{
	unsigned char x=0;
	DQ = 1; //DQ��λ
	delay2(8); //������ʱ
	DQ = 0; //��Ƭ����DQ����
	delay2(80); //��ȷ��ʱ ���� 480us
	DQ = 1; //��������
	delay2(14);
	x=DQ; //������ʱ�� ���x=0���ʼ���ɹ� x=1���ʼ��ʧ��
	delay2(20);
}
//DS18B20��һ���ֽ�
uchar ReadOneChar(void)
{
	unsigned char i=0;
	unsigned char dat = 0;
	for (i=8;i>0;i--){
		DQ = 0; // �������ź�
		dat>>=1;
		DQ = 1; // �������ź�
		if(DQ)  dat|=0x80;
		delay2(4);
		}
	return(dat);
}

//DS18B20дһ���ֽ�
void WriteOneChar(unsigned char dat)
{
	unsigned char i=0;
	for (i=8; i>0; i--){
		DQ = 0;
		DQ = dat&0x01;
		delay2(5);
		DQ = 1;
		dat>>=1;
		}
}
//DS18B20��ȡ�¶�
uint ReadTemperature(void)
{
	unsigned char a=0;
	unsigned char b=0;
	unsigned int t=0;
	float tt=0;
	Init_DS18B20();
	WriteOneChar(0xCC); // ����������кŵĲ���
	WriteOneChar(0x44); // �����¶�ת��
	Init_DS18B20();
	WriteOneChar(0xCC); //����������кŵĲ���
	WriteOneChar(0xBE); //��ȡ�¶ȼĴ����ȣ����ɶ�9���Ĵ����� ǰ���������¶�
	a=ReadOneChar();
	b=ReadOneChar();
	t=b;
	t<<=8;
	t=t|a;
	tt=t*0.0625; //���¶ȵĸ�λ���λ�ϲ�
	t= tt*10+0.5; //�Խ������4��5��
	return(t);
}
//*******************��ʾ�¶�
void display_temperature()
{
	uchar i;
	uint ntemp;
	EA=0;
	P0=0X00;
	dula=1;
	wela=0;
	delay1ms();
	dula=0;
	wela=0;
	delay1ms();
	delay1ms();
	delay1ms();
	lcd_init();
	P0 = 0xff;
	write_1602com(0x06); //���������1
write_1602com(0x82);
	i=0;
	while(dis11[i]!='\0')
		{
			write_1602dat(dis11[i]);	 
			i++;
		}
		while(free)
			{
				ntemp=ReadTemperature();
				write_1602com(0xC2);
				dis_tem[0] = mun_to_char[ntemp/100];
				dis_tem[1] = mun_to_char[ntemp%100/10];
				dis_tem[3] = mun_to_char[ntemp%10];
				for(i=0;i<4;i++)
					{
						write_1602dat(dis_tem[i]);
					}
				write_1602dat(0x0df);
				write_1602dat(0x43);
				write_1602com(0xC2);
				scanKeyboard0();
				if(ntemp>300)
					{
						fault_led();
					}
			}
}