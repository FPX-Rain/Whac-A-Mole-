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
	static u8 key_x = 0;//0,û���κΰ������£�1~9��1~9�Ű�������
	u8 key = 0;
	tp_dev.scan(0);
	if (tp_dev.sta & TP_PRES_DOWN)			//������������
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
	NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);//����ϵͳ�ж����ȼ�����2
	delay_init(168);  //��ʼ����ʱ����
	uart_init(115200);
	LED_Init();					//��ʼ��LED 
	LCD_Init();					//LCD��ʼ��  
	KEY_Init();					//������ʼ�� 
	RNG_Init();
	W25QXX_Init();				//��ʼ��W25Q128
	tp_dev.init();				//��ʼ��������
	my_mem_init(SRAMIN);		//��ʼ���ڲ��ڴ�� 
	my_mem_init(SRAMCCM); //?????64,????500,?????1s	
	//��ʼ��CCM�ڴ��
	usmart_dev.init(84);		//��ʼ��USMART
RESTART:
	POINT_COLOR = RED;
	while (font_init()) 				//����ֿ�
	{
		LCD_ShowString(60, 50, 200, 16, 16, "Font Error!");
		delay_ms(200);
		LCD_Fill(60, 50, 240, 66, WHITE);//�����ʾ	     
	}
	Show_Str(30, 105, 200, 16, "����:    ", 16, 0);
	Show_Str(30, 125, 200, 16, "�÷�:     :  ", 16, 0);
	LCD_Fill(30, 145, 210, 32, WHITE);	//���֮ǰ����ʾ
	py_load_ui(30, 195);
	sum_soucre = 0;
	while (1) {
		u8 key,key_num;
		int soucre,sum_soucre;
		delay_ms(10);
		Show_Str(30, 105, 200, 16, "����:    ", 16, 0);
		LCD_Fill(30 + 40, 125, 30 + 40 + 48, 130 + 16, WHITE);	//���֮ǰ����ʾ
		LCD_Fill(30 + 40, 145, 30 + 200, 145 + 48, WHITE);	//���֮ǰ����ʾ
		key_num = RNG_Get_RandomRange(1,9);
		LCD_ShowNum(30 + 144, 105, key_num, 10, 16);
		LCD_ShowNum(30 + 144, 125, sum_soucre, 10, 16);
		soucre=100;
		key = py_get_keynum(30, 195);
		while(key==0){
		if(soucre==0){
				sum_soucre = 0;
				LCD_ShowNum(30 + 144, 125, sum_soucre, 10, 16);
				Show_Str(30, 145, 210, 32, "������:", 16, 0);
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
				Show_Str(30, 145, 210, 32, "������:", 16, 0);
				delay_ms(1000);
}}}}