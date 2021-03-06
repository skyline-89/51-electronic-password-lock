#include <reg52.h>
#include <intrins.h>
#define uchar unsigned char  
#define uint unsigned int 
#define OP_WRITE 0xa0          // 器件地址以及写入操作 
#define OP_READ  0xa1           // 器件地址以及读取操作 
#define delayNOP(); {_nop_();_nop_();_nop_();_nop_();}; 
//************************定义端口*************************
sbit SDA = P2^0; //数据线
sbit SCL = P2^1; //时钟线
sbit dula=P2^6;
sbit wela=P2^7;
sbit alarm =P2^3;
sbit LCD_HH=P3^1;	
sbit LCD_EN=P3^4;	
sbit LCD_RS=P3^5;
sbit LCD_RW=P3^6;
sbit LCD_RD = P3^7;
//************************定义数组及全局变量*************************
uchar v,w,mark3,free;
uchar judge;
uchar code initial_pass[6]={6,1,2,3,4,5};	//初始密码
//***************存储1602显示的数据*******************************
uchar code mun_to_char[] = {"0123456789ABCDEF"}; /*定义数字跟ASCII码的关系*/
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
//*****************声明操作24C02的函数*****************
void delayms(uint ms);
void start(); 
void stop(); 
void change_passwords();
bit match_passwords();
void write_byte(uchar addr, uchar write_data);
uchar read_random(uchar random_addr); 
uchar data_in(); 
bit data_out(uchar write_data); 
//*****************************************进程
uchar passwords[8];//临时存储密码
bit aleady_confirm;//是否点击确认
uchar pw_length;//用户已经输入的密码长度
uchar temp,num;//用于矩阵按键识别
void manager();//管理员开锁
void process1();//输入密码
void process2();//选择
void over_op();//确认按键后的操作
void over_op2();//密码正确后的操作
void delay(uchar);//延迟函数
//**************************按键扫描
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
//******************声光********************************** 
uchar chaoshi_led();
void open_led(void);
void shanshuo_led();
void fault_led();
void chaoshi_jingbao();
//*******************延迟函数****************************
void delay1ms(void);
void delay5ms(void);
void delay50ms(void);
void delay500ms(void);
void delay1s500ms(void);
void delay2s500ms(void);
void delay5s();
void delay10s(void);
/*******************18B20***************************************/
sbit DQ =P2^2;  //定义DS18B20通信端口
void Init_DS18B20();
uchar ReadOneChar();
void WriteOneChar(unsigned char dat);
uint ReadTemperature();
void display_temperature();
void delay2(unsigned int i);
//***********************主函数***************************
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
	//进程一，输入密码
	loop1:
	display_temperature();
	mark=0;
	loop2:
	display_shuru();
	attempts=0;
	do{
		//三次开锁失败操作
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
		//输入错误的操作
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
		//输入密码
		process1();
		write_byte(0,pw_length);
		for(j=1;j<=pw_length;j++)
			write_byte(j,passwords[j-1]);
		//匹配密码
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
	//修改成功，返回启动界面
}
//**********************************************************
bit match_passwords()
//密码匹配
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
// 在指定地址addr处写入数据write_data 
{ 
	SDA = 1; 
	SCL = 1;
	start(); 
	data_out(OP_WRITE); 
	data_out(addr); 
	data_out(write_data); 
	stop(); 
	delayms(10);          // 写入周期 
} 


uchar read_random(uchar random_addr) 
// 在指定地址读取存储的字节
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
// 从AT24C02移出数据到MCU 
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
// 从MCU移出数据到AT24C02 
{ 
	uchar i; 
	bit ack_bit; 
	for(i = 0; i < 8; i++)   // 循环移入8个位 
	{ 
		SDA = (bit)(write_data & 0x80); 
		_nop_(); 
		SCL = 1; 
		delayNOP(); 
		SCL = 0; 
		write_data <<= 1; 
	} 
	SDA = 1;                     // 读取应答 
	delayNOP(); 
	SCL = 1; 
	delayNOP(); 
	ack_bit = SDA; 
	SCL = 0; 
	return ack_bit;            // 返回AT24C02应答位 
} 
 
void start() 
 //开始位 
{ 
	SDA = 1; 
	SCL = 1; 
	delayNOP(); 
	SDA = 0; 
	delayNOP(); 
	SCL = 0; 
} 

void stop() 
 // 停止位 
{ 
	SDA = 0; 
	delayNOP(); 
	SCL = 1; 
	delayNOP(); 
	SDA = 1; 
} 


//**********************************************************
void process1()
//输入密码操作
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
//选择开锁还是修改密码
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
	if(num==10)//执行退格操作
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
	else if(num==11||pw_length==8)//执行确认操作
	{
		aleady_confirm=1;
	}

	else //将输入的数字保存
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
	if(num==12)//选项一开锁
	{
		xuanze1();
		delay50ms();
		open_led();
	}
	if(num==13)//选项二修改密码
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
//-------------------测忙---------------
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
//--------------写指令--------------
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
//--------------写数据-------------
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
//---------------初始化----------------------
void lcd_init(void)
{
delay5ms();
delay5ms();
delay5ms();
write_1602com(0x38); //16*2显示，5*7点阵，8位数据 ，设置工作模式
delay5ms();
delay5ms(); 
write_1602com(0x38); 
delay5ms();
delay5ms();
write_1602com(0x38); 
delay5ms(); 
write_1602com(0x0c); //显示开，关光标 
delay5ms(); 
write_1602com(0x06); //向右移动光标 ，整屏不移动
delay5ms(); 
write_1602com(0x01); //清除LCD的显示内容 
delay5ms(); 
}
//----------------提示输入密码显示--------------
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
write_1602com(0x80);		//显示位置为第一行第一个字符
i=0;
while(dis1[i]!='\0')
{
write_1602dat(dis1[i]);	  //显示“INPUT PASSWORD”
i++;
}
}										                             
//************显示“ERROR”***********
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
write_1602com(0x80);		//显示位置为第一行第一个字符
i=0;
while(dis3[i]!='\0')
{
write_1602dat(dis3[i]);	  //显示“ERROR”
i++;
}				                                          
}


//****************显示“SUCCESSFUL”***
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
write_1602com(0x82);	//显示位置为第一行第三个字符
i=0;
while(dis6[i]!='\0')
{
write_1602dat(dis6[i]);	 
i++;
}				                                          
}
//***********显示再次输入************
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
write_1602com(0x80);		//显示位置为第一行第1个字符
i=0;
while(dis9[i]!='\0')
{
write_1602dat(dis9[i]);	 
i++;
}				                                          
}
//**************显示密码不同界面***********
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


//*******************选择界面**********
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
//**************选择第二行的情况(左键）*******
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
//********第二行右键**********
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

//***********改密界面************
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
//**********锁定显示***********
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
write_1602com(0x80);	//显示位置为第一行第1个字符
i=0;
while(dis10[i]!='\0')
{
write_1602dat(dis10[i]);	 
i++;
}				                                          
}
//---------------------------输密码显示“ * ”

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
//退格
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



//******************声音和灯光************
//开锁灯，依次亮后闪一长亮
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
//错三次警报灯闪烁;
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
//密码错误,后四闪
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
//**********所有的延时子程序*************
void delayms(uint ms)  
// 1ms延时子程序 
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
//延时1ms;
void delay1ms(void)  
{
    unsigned char a,b,c;
    for(c=1;c>0;c--)
        for(b=142;b>0;b--)
            for(a=2;a>0;a--);
}
//延时5ms
void delay5ms(void)  
{
    unsigned char a,b;
    for(b=19;b>0;b--)
        for(a=130;a>0;a--);
}
//延时50ms;
void delay50ms(void)   
{
    unsigned char a,b;
    for(b=173;b>0;b--)
        for(a=143;a>0;a--);
}
//延时500ms
void delay500ms(void)   
{
    unsigned char a,b,c;
    for(c=23;c>0;c--)
        for(b=152;b>0;b--)
            for(a=70;a>0;a--);
}
//延时1.5s;
void delay1s500ms(void)   
{
    unsigned char a,b,c;
    for(c=127;c>0;c--)
        for(b=96;b>0;b--)
            for(a=60;a>0;a--);
}
//延时2.5s
void delay2s500ms(void)   
{
    unsigned char a,b,c,n;
    for(c=229;c>0;c--)
        for(b=214;b>0;b--)
            for(a=24;a>0;a--);
    for(n=1;n>0;n--);
    _nop_();  
}
//延时10s;
void delay10s(void) 
{
    unsigned char a,b,c;
    for(c=191;c>0;c--)
        for(b=189;b>0;b--)
            for(a=137;a>0;a--);
    _nop_();   
}
//延时5s;
void delay5s()   
{
    unsigned char a,b,c;
    for(c=165;c>0;c--)
        for(b=100;b>0;b--)
            for(a=150;a>0;a--);
    _nop_(); 
    _nop_(); 
}
//************************按键超时
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
//DS18B20延时函数
void delay2(unsigned int i)
{
	while(i--);
}
//DS18B20初始化函数
void Init_DS18B20(void)
{
	unsigned char x=0;
	DQ = 1; //DQ复位
	delay2(8); //稍做延时
	DQ = 0; //单片机将DQ拉低
	delay2(80); //精确延时 大于 480us
	DQ = 1; //拉高总线
	delay2(14);
	x=DQ; //稍做延时后 如果x=0则初始化成功 x=1则初始化失败
	delay2(20);
}
//DS18B20读一个字节
uchar ReadOneChar(void)
{
	unsigned char i=0;
	unsigned char dat = 0;
	for (i=8;i>0;i--){
		DQ = 0; // 给脉冲信号
		dat>>=1;
		DQ = 1; // 给脉冲信号
		if(DQ)  dat|=0x80;
		delay2(4);
		}
	return(dat);
}

//DS18B20写一个字节
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
//DS18B20读取温度
uint ReadTemperature(void)
{
	unsigned char a=0;
	unsigned char b=0;
	unsigned int t=0;
	float tt=0;
	Init_DS18B20();
	WriteOneChar(0xCC); // 跳过读序号列号的操作
	WriteOneChar(0x44); // 启动温度转换
	Init_DS18B20();
	WriteOneChar(0xCC); //跳过读序号列号的操作
	WriteOneChar(0xBE); //读取温度寄存器等（共可读9个寄存器） 前两个就是温度
	a=ReadOneChar();
	b=ReadOneChar();
	t=b;
	t<<=8;
	t=t|a;
	tt=t*0.0625; //将温度的高位与低位合并
	t= tt*10+0.5; //对结果进行4舍5入
	return(t);
}
//*******************显示温度
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
	write_1602com(0x06); //光标向右移1
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