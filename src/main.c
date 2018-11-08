/**
  ******************************************************************************
  * @file    main.c
  * @author  Ac6
  * @version V1.0
  * @date    01-December-2013
  * @brief   Default main function.
  ******************************************************************************
*/

#include "stm32f4xx.h"
#include "stdlib.h"
/* Include my libraries here */
//#include "defines.h"
#include "tm_stm32f4_usart.h"
#include "elevator_lcd.h"
#include "elevator.h"
#include <string.h>

#define STOP_TIME 3000

int main(void)
{
	RCC_AHB1PeriphClockCmd(RCC_AHB1Periph_GPIOA | RCC_AHB1Periph_GPIOB | RCC_AHB1Periph_GPIOC | RCC_AHB1Periph_GPIOD, ENABLE);
	SysTick_Config(SystemCoreClock/ 1000);

	TM_USART_Init(USART2, TM_USART_PinsPack_1, 9600);
	TM_USART_Puts(USART2, "start elevator\n");

	init();
	ADCInit();
	system_on();
	flash_read();
	lcd_update(0);
	while(1){
		while(1){  // elevator select floor
			input_memory(input_key());
			App_ReadData();
			led_on(elevator_state);
			fnd_display(elevator_state, elevator_state);
			for(int i = elevator_state; i<6 && i >= 1; i = i + elevator_direction){
				if(elevator_floor[1][i] || elevator_floor[elevator_direction+1][i]){
					elevator_dest = i;
					break;
				}
			}
			if(elevator_dest) break;

			for(int i = elevator_state; i<6 && i >= 1; i = i + elevator_direction){
				if(elevator_floor[-elevator_direction+1][i]){
					elevator_dest = i;
					break;
				}
			}

			if(elevator_dest) break;
			elevator_direction = -elevator_direction;
		}

		if(elevator_state != elevator_dest){  // elevator move
			elevator_state += elevator_direction;
			int future = msec + 2000;
			lcd_update(adc);
			App_SendData(elevator_state, elevator_direction);
			while(future > msec){
				int key = input_key();
				if(KM_EMERG == key){
					while(1){
						char* str = ReadUartStr();
						int input;
						sprintf(buf, "%dF:",elevator_state);
						lcd_printxy(0, 0, buf);
						lcd_printxy(0, 1, "  EMERGENCY!!!   ");
						buzzer_on();
						if(str){
							sscanf(str, "%d", &input);
							if(input == 9) break;
						}
					}
				}
				led_on(elevator_state);
				fnd_display(elevator_state,elevator_state);
			}
		}

		if(elevator_dest != elevator_state) continue;
		App_SendData(elevator_state, app_direct);

		int door_time = 500;
		while(door_time > 0){  // door system

			adc = ADC_Read();
			adc = adc / 31;

			motor_on(2);  // door open
			door_char = 'O';
			int motor_future = msec + door_time;
		    while(motor_future > msec){
		    	int key = input_key();
		    }

		    motor_on(0);  // door stop
		    motor_future = msec + STOP_TIME;

			if(adc >= 100){
				while(1){
					adc = ADC_Read();
					adc = adc / 31;

					sprintf(buf, "%dF:",elevator_state);
					lcd_printxy(0, 0, buf);
					lcd_printxy(0, 1, " OVER WEIGHT!!! ");
					buzzer_on();

					if(adc < 100) break;
				}
			}
			lcd_update(adc);

		    while(motor_future > msec){
		    	int key = input_key();
		    	if(key == KM_OPEN_) motor_future = msec + STOP_TIME;
		    	if(GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2) == 1) motor_future = msec + STOP_TIME; // photo
		    	if(key == KM_CLOSE) break;
		    }

		    motor_on(1);  // door close
		    door_char = 'C';
		    motor_future = msec + 500;
		    motor_current_time = msec;
		    door_time = 0;
		    while(motor_future > msec){
		    	int key = input_key();
		    	if(key == KM_OPEN_ || GPIO_ReadInputDataBit(GPIOD,GPIO_Pin_2) == 1){
		    		door_time = msec - motor_current_time;
		    		break;
		    	}
		    }
		    motor_on(0);
		    lcd_update(adc);
		}

		App_SendData(elevator_state, 0);
		elevator_floor[1][elevator_dest] = 0; // 해당 층 이동한 뒤에 초기화
		elevator_floor[-elevator_direction+1][elevator_dest] = 0;
		elevator_floor[elevator_direction+1][elevator_dest] = 0;
		elevator_dest = 0;


	}
}
