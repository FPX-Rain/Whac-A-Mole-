#include "sys.h"
#include "delay.h"  
#include "usart.h"   
#include "led.h"
#include "lcd.h"
#include "key.h"  
#include "malloc.h" 
#include "w25qxx.h"       
#include "fontupd.h"
#include "text.h"   
#include "pyinput.h"
#include "touch.h"	
#include "string.h"  
#include "usmart.h"
#include "stdlib.h"
#include "rng.h"
const u8* kbd_tbl[9] = { "1","2","3","4","5","6","7","8","9", };
const u8* kbs_tbl[9] = { " "," "," "," "," "," "," "," "," ", };
void py_load_ui(u16 x, u16 y)
{
	u16 i;
	POINT_COLOR = RED;
	LCD_DrawRectangle(x, y, x + 180, y + 120);
	LCD_DrawRectangle(x + 60, y, x + 120, y + 120);
	LCD_DrawRectangle(x, y + 40, x + 180, y + 80);
	POINT_COLOR = BLUE;
	for (i = 0; i < 9; i++)
	{
		Show_Str_Mid(x + (i % 3) * 60, y + 4 + 40 * (i / 3), (u8*)kbd_tbl[i], 16, 60);
		Show_Str_Mid(x + (i % 3) * 60, y + 20 + 40 * (i / 3), (u8*)kbs_tbl[i], 16, 60);
	}
}
void py_key_staset(u16 x, u16 y, u8 keyx, u8 sta)
{
	u16 i = keyx / 3, j = keyx % 3;
	if (keyx > 8)return;
	if (sta)LCD_Fill(x + j * 60 + 1, y + i * 40 + 1, x + j * 60 + 59, y + i * 40 + 39, GREEN);
	else LCD_Fill(x + j * 60 + 1, y + i * 40 + 1, x + j * 60 + 59, y + i * 40 + 39, WHITE);
	Show_Str_Mid(x + j * 60, y + 4 + 40 * i, (u8*)kbd_tbl[keyx], 16, 60);
	Show_Str_Mid(x + j * 60, y + 20 + 40 * i, (u8*)kbs_tbl[keyx], 16, 60);
}
u8 py_get_keynum(u16 x, u16 y)
{
	u16 i, j;
	static u8 key_x = 0;//0,没有任何按键按下；1~9，1~9号按键按下
	u8 key = 0;
	tp_dev.scan(0);
	if (tp_dev.sta & TP_PRES_DOWN)			//触摸屏被按下
	{
		for (i = 0; i < 3; i++)
		{
			for (j = 0; j < 3; j++)
			{
				if (tp_dev.x[0]<(x + j * 60 + 60) && tp_dev.x[0]>(x + j * 60) && tp_dev.y[0]<(y + i * 40 + 40) && tp_dev.y[0]>(y + i * 40))
				{
					key = i * 3 + j + 1;
					break;
				}
			}
			if (key)
			{
				if (key_x == key)key = 0;
				else
				{
					py_key_staset(x, y, key_x - 1, 0);
					key_x = key;
					py_key_staset(x, y, key_x - 1, 1);
				}
				break;
			}
		}
	}
	
	else if (key_x)
	{
		py_key_staset(x, y, key_x - 1, 0);
		key_x = 0;
	}
	return key;
}
int main(void) {
	u8 key_num,soucre,sum_soucre;
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//设置系统中断优先级分组2
	delay_init(168);  //初始化延时函数
	uart_init(115200);
	LED_Init();					//初始化LED 
	LCD_Init();					//LCD初始化  
	KEY_Init();					//按键初始化 
	RNG_Init();
	W25QXX_Init();				//初始化W25Q128
	tp_dev.init();				//初始化触摸屏
	my_mem_init(SRAMIN);		//初始化内部内存池 
	my_mem_init(SRAMCCM); //?????64,????500,?????1s	
	//初始化CCM内存池
	usmart_dev.init(84);		//初始化USMART
RESTART:
	POINT_COLOR = RED;
	while (font_init()) 				//检查字库
	{
		LCD_ShowString(60, 50, 200, 16, 16, "Font Error!");
		delay_ms(200);
		LCD_Fill(60, 50, 240, 66, WHITE);//清除显示	     
	}
	Show_Str(30, 105, 200, 16, "输入:    ", 16, 0);
	Show_Str(30, 125, 200, 16, "得分:     :  ", 16, 0);
	LCD_Fill(30, 145, 210, 32, WHITE);	//清除之前的显示
	py_load_ui(30, 195);
	sum_soucre = 0;
	while (1) {
		u8 key,key_num;
		int soucre,sum_soucre;
		delay_ms(10);
		Show_Str(30, 105, 200, 16, "输入:    ", 16, 0);
		LCD_Fill(30 + 40, 125, 30 + 40 + 48, 130 + 16, WHITE);	//清除之前的显示
		LCD_Fill(30 + 40, 145, 30 + 200, 145 + 48, WHITE);	//清除之前的显示
		key_num = RNG_Get_RandomRange(1,9);
		LCD_ShowNum(30 + 144, 105, key_num, 10, 16);
		LCD_ShowNum(30 + 144, 125, sum_soucre, 10, 16);
		soucre=100;
		key = py_get_keynum(30, 195);
		while(key==0){
		if(soucre==0){
				sum_soucre = 0;
				LCD_ShowNum(30 + 144, 125, sum_soucre, 10, 16);
				Show_Str(30, 145, 210, 32, "你输了:", 16, 0);
				delay_ms(1000);
			soucre=100;
		}
			soucre=soucre-1;
		key = py_get_keynum(30, 195);
		delay_ms(50);
		}
		if(key){
			if(key==key_num){
				sum_soucre += soucre;
				LCD_ShowNum(30 + 144, 125, sum_soucre, 10, 16);
			}
			else{
				sum_soucre = 0;
				LCD_ShowNum(30 + 144, 125, sum_soucre, 10, 16);
				Show_Str(30, 145, 210, 32, "你输了:", 16, 0);
				delay_ms(1000);
}}}}