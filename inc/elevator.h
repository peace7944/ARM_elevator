/*
 * elevator.h
 *
 *  Created on: 2018. 10. 30.
 *      Author: USER
 */
#include "stm32f4xx.h"
#include <string.h>

#ifndef ELEVATOR_H_
#define ELEVATOR_H_

#define FLASH_DATA_SECTOR 0x080E0000

#define KM_INT_1 0b00010001
#define KM_INT_2 0b00010010
#define KM_INT_3 0b00010100
#define KM_INT_4 0b00011000
#define KM_INT_5 0b00100001
#define KM_CLOSE 0b00100010
#define KM_OPEN_ 0b00100100
#define KM_EMERG 0b00101000
#define KM_TITLE 0b01000001
#define KM_OUT_1 0b01000010
#define KM_OUT_2 0b01000100
#define KM_OUT_3 0b01001000
#define KM_OUT_4 0b10000001
#define KM_OUT_5 0b10000010
#define KM_UP___ 0b10000100
#define KM_DOWN_ 0b10001000

volatile int msec = 0;
int Num7Segs[15] = {0b11000000, 0b11111001, 0b10100100, 0b10110000, 0b10011001, 0b10010010,
               0b10000011, 0b11111000, 0b10000000, 0b10011000};
unsigned int key = 0;
char buf[18] = {0, };
char buf2[18] = {0, };

int elevator_floor[3][6] = {0, }; // ext - down 0, int 1, ext - up 3
int elevator_direction = 1, elevator_state = 1, elevator_dest = 0;
int motor_current_time = 0, adc = 0;
int key_num_updown = 0;

char door_char = 0;
char inputbuf[255];
int bufsize = 0;

int title_floor = 1;
char title_input[6][20] = {0,};

int app_direct = 0;

void delay_ms(int ms){
	volatile int future = ms + msec;
	while(future > msec);
}

void SysTick_Handler(void){ // 라이브러리에 표준으로 지정된 함수임
	msec++;
}

void init(){
	GPIO_InitTypeDef GPIO_Initstruct;

	GPIO_Initstruct.GPIO_Pin = GPIO_Pin_1 | GPIO_Pin_6 | GPIO_Pin_7;// 1 = buzzer, 6, 7 = motor
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Initstruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Initstruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Initstruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOB, &GPIO_Initstruct);

	GPIO_Initstruct.GPIO_Pin = GPIO_Pin_0 | GPIO_Pin_1 | GPIO_Pin_2 | GPIO_Pin_3 | GPIO_Pin_4 //LED
			| GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11 | GPIO_Pin_12 | GPIO_Pin_13 | GPIO_Pin_14 | GPIO_Pin_15;// FND
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Init(GPIOC, &GPIO_Initstruct);

	GPIO_Initstruct.GPIO_Pin = GPIO_Pin_8 | GPIO_Pin_9 | GPIO_Pin_10 | GPIO_Pin_11;// key_row
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_OUT;
	GPIO_Initstruct.GPIO_OType = GPIO_OType_PP;
	GPIO_Initstruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_Initstruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_Init(GPIOA, &GPIO_Initstruct);

	GPIO_Initstruct.GPIO_Pin = GPIO_Pin_4 | GPIO_Pin_5 | GPIO_Pin_6 | GPIO_Pin_7; // key_col
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Initstruct.GPIO_PuPd = GPIO_PuPd_DOWN;
	GPIO_Init(GPIOA, &GPIO_Initstruct);

	GPIO_Initstruct.GPIO_Pin = GPIO_Pin_2; // photo
	GPIO_Initstruct.GPIO_Mode = GPIO_Mode_IN;
	GPIO_Init(GPIOD, &GPIO_Initstruct);

	lcd_init();
	lcd_clear();

	GPIO_SetBits(GPIOC, 0x001f);
}

void ADCInit(){
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_ADC1, ENABLE);
	GPIO_InitTypeDef GPIO_InitStruct;
	GPIO_InitStruct.GPIO_Mode = GPIO_Mode_AN;
	GPIO_InitStruct.GPIO_PuPd = GPIO_PuPd_NOPULL;
	GPIO_InitStruct.GPIO_Speed = GPIO_Speed_100MHz;
	GPIO_InitStruct.GPIO_Pin = GPIO_Pin_0;
	GPIO_Init(GPIOB, &GPIO_InitStruct);

	ADC_InitTypeDef ADC_InitStruct;
	ADC_InitStruct.ADC_ContinuousConvMode = DISABLE;
	ADC_InitStruct.ADC_DataAlign = ADC_DataAlign_Right;
	ADC_InitStruct.ADC_ExternalTrigConv = DISABLE;
	ADC_InitStruct.ADC_ExternalTrigConvEdge = ADC_ExternalTrigConvEdge_None;
	ADC_InitStruct.ADC_NbrOfConversion = 1;
	ADC_InitStruct.ADC_Resolution = ADC_Resolution_12b;
	ADC_Init(ADC1, &ADC_InitStruct);

	ADC_Cmd(ADC1, ENABLE);
	ADC_RegularChannelConfig(ADC1, ADC_Channel_8, 1, ADC_SampleTime_84Cycles);
}

int ADC_Read(){
	ADC_SoftwareStartConv(ADC1);
	while(!ADC_GetFlagStatus(ADC1, ADC_FLAG_EOC));
	return ADC_GetConversionValue(ADC1);
}

void fnd_display(int num1, int num2){
	for(int i=0;i<2;i++){
		GPIO_SetBits(GPIOC, 0b1111111100000000);
		GPIO_ResetBits(GPIOC, 0b1111111100000000);

		if(i == 0) GPIO_SetBits(GPIOC, Num7Segs[num1] << 8);
		if(i == 1) GPIO_SetBits(GPIOC, Num7Segs[num2] << 8);
		delay_ms(1);
	}
}

void buzzer_on(){
	int future = msec + 1000;
	while(future > msec){
		GPIO_SetBits(GPIOB, GPIO_Pin_1);
	}
	int future2 = msec + 1000;
	while(future2 > msec){
		GPIO_ResetBits(GPIOB, GPIO_Pin_1);
	}
}

void led_on(int floor){
	GPIO_SetBits(GPIOC, 0b0000000000011111);
	if(floor == 1) GPIO_ResetBits(GPIOC, GPIO_Pin_0);
	if(floor == 2) GPIO_ResetBits(GPIOC, GPIO_Pin_1);
	if(floor == 3) GPIO_ResetBits(GPIOC, GPIO_Pin_2);
	if(floor == 4) GPIO_ResetBits(GPIOC, GPIO_Pin_3);
	if(floor == 5) GPIO_ResetBits(GPIOC, GPIO_Pin_4);
}

void motor_on(int action){
	if(action == 1){
		GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
		GPIO_SetBits(GPIOB, GPIO_Pin_6);
	}
	if(action == 2){
		GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
		GPIO_SetBits(GPIOB, GPIO_Pin_7);
	}
	if(action == 0){
		GPIO_ResetBits(GPIOB, GPIO_Pin_6 | GPIO_Pin_7);
	}
}

unsigned int getkey(){
	int i;
	for(i=0;i<4;i++){
		GPIO_ResetBits(GPIOA, 0b1111 << 8);
		GPIO_SetBits(GPIOA, 1 << i << 8);
		delay_ms(1);
		unsigned int key = (GPIO_ReadInputData(GPIOA) >> 4) & 0b1111; // 0b1111 = 0b0000000000001111
		if(key != 0) return 1 << 4 << i  | key;
	}
	return 0;
}

int input_key(){
	int key = getkey();
	int key_num = 0;

	if(key != 0){
		if(KM_OUT_1 == key) key_num_updown = 1;
		else if(KM_OUT_2 == key) key_num_updown = 2;
		else if(KM_OUT_3 == key) key_num_updown = 3;
		else if(KM_OUT_4 == key) key_num_updown = 4;
		else if(KM_OUT_5 == key) key_num_updown = 5;

		if(KM_UP___ == key) elevator_floor[2][key_num_updown] = 1;
		else if(KM_DOWN_ == key) elevator_floor[0][key_num_updown] = 1;

		if(KM_INT_1 == key) key_num = 1;
		else if(KM_INT_2 == key) key_num = 2;
		else if(KM_INT_3 == key) key_num = 3;
		else if(KM_INT_4 == key) key_num = 4;
		else if(KM_INT_5 == key) key_num = 5;
	}
	if(key > 0 && key_num > 0) elevator_floor[1][key_num] = 1;

	return key;
}

void system_on(){
	int future = msec + 1000;
	while(future > msec){
		sprintf(buf,"Elevator System ");
		sprintf(buf2,"       LOADING");
		lcd_printxy(0,0, buf);
		lcd_printxy(0,1, buf2);
		motor_on(2);
		fnd_display(1,1);
	}

	future = msec + 1000;
	while(future > msec){
		sprintf(buf2,"       LOADING!");
		lcd_printxy(0,0, buf);
		lcd_printxy(0,1, buf2);
		motor_on(0);
		fnd_display(0,0);
	}

	future = msec + 1000;
	while(future > msec){
		sprintf(buf2,"       LOADING!!");
		lcd_printxy(0,0, buf);
		lcd_printxy(0,1, buf2);
		motor_on(1);
		fnd_display(0,0);
	}
	motor_on(0);
	lcd_clear();
}

char* ReadUartStr()
{
	int c;
	do {
		c = TM_USART_Getc(USART2);
		if (c) {
			if (c=='\n')
			{
				inputbuf[bufsize] = 0;
				bufsize=0;
				return inputbuf;
			}
			inputbuf[bufsize++] = c;
		}
	} while (c);
	return 0;
}

void program_data(uint32_t dest, uint8_t *src, uint32_t size)
{
	while(size--)
	{
		FLASH_ProgramByte(dest++, *src++);
	}
}

void flash_write(){
	FLASH_Unlock();  // 잠금 해제
	FLASH_ClearFlag(FLASH_FLAG_EOP | FLASH_FLAG_OPERR | FLASH_FLAG_WRPERR | FLASH_FLAG_PGAERR | FLASH_FLAG_PGPERR | FLASH_FLAG_PGSERR);  //에러 플래그들 초기화함.
	FLASH_EraseSector(FLASH_Sector_11, VoltageRange_3);
	program_data(FLASH_DATA_SECTOR + 40, title_input, sizeof(title_input));
	FLASH_Lock();
}

void flash_read(){
 	memcpy(title_input, (char*)(FLASH_DATA_SECTOR + 40), sizeof(title_input));
}

void input_memory(int t_key){
	int flag = 1;

	if(KM_TITLE == t_key){
		TM_USART_Puts(USART2, ">>> INPUT FLOOR TITLE\n");
		while(1){
			char* str = ReadUartStr();
			char tmp[18] = {0,};

			if(flag){
				sprintf(tmp,"%dF:", title_floor);
				TM_USART_Puts(USART2, tmp);
				flag = 0;
			}
			if(str){
				flag = 1;
				lcd_clear();
				sscanf(str, "%s", title_input[title_floor]);
				sprintf(buf, "%dF:%s",title_floor, title_input[title_floor++]);

				lcd_printxy(0, 0, buf);
				TM_USART_Puts(USART2, "\n");

				if(title_floor == 6){
					TM_USART_Puts(USART2, "END\n");
					break;
				}
			}
		}
		flash_write();
	}
}

void lcd_update(int adc){
	char num[5] = {0, };
	lcd_clear();
	for(int i=1;i<6;i++){
		if(elevator_floor[0][i] || elevator_floor[1][i] || elevator_floor[2][i]) num[i-1] = i + 48;
		else num[i-1] = 95;
	}
	sprintf(buf, "%dF:%s",elevator_state, title_input[elevator_state]);
	sprintf(buf2, "W: %dkg %c %s", adc, door_char, num);
	lcd_printxy(0, 0, buf);
	lcd_printxy(0, 1, buf2);
}

void App_SendData(int floor, int direct){
	char tmp[3] = {0,};
	tmp[0] = floor + 48;
	if(direct == 1) tmp[1] = '1';
	else if(direct == -1) tmp[1] = '2';
	else tmp[1] = '0';

	TM_USART_Puts(USART2, tmp);
}

void App_ReadData(){
	int received = TM_USART_Getc(USART2);
	if(received){
		if(received == 49){
			elevator_floor[2][1] = 1;
			app_direct = 1;
		}
		else if(received == 51){
			elevator_floor[2][2] = 1;
			app_direct = 1;
		}
		else if(received == 52){
			elevator_floor[0][2] = 1;
			app_direct = -1;
		}
		else if(received == 53){
			elevator_floor[2][3] = 1;
			app_direct = 1;
		}
		else if(received == 54){
			elevator_floor[0][3] = 1;
			app_direct = -1;
		}
		else if(received == 55){
			elevator_floor[2][4] = 1;
			app_direct = 1;
		}
		else if(received == 56){
			elevator_floor[0][4] = 1;
			app_direct = -1;
		}
		else if(received == 50){
			elevator_floor[0][5] = 1;
			app_direct = -1;
		}
	}
}

#endif /* ELEVATOR_H_ */
