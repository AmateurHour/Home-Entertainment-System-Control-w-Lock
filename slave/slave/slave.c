#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <math.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>
#include <avr/portpins.h>
#include <avr/pgmspace.h>
#include <avr/delay.h>

//FreeRTOS include files
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "lcd.h"
#include "bit.h"
#include "usart_ATmega1284.h"



const int QUARTER_PHASE = 90/5.625*64;
const int HALF_PHASE = 180/5.625*64;
const int FULL_PHASE = 360/5.625*64;
static char unlock = 0;
static char unlocked= 0;
char bluetooth = 0;
char logged_in = 0;
char tv = 0;
enum stepperState{WAIT, A, AB, B, BC, C, CD, D, DA} step_state;

enum servoState{LOCK, UNLOCK, OPEN} servo_state;
	
	void Wait()
	{
		uint8_t i;
		for(i=0;i<50;i++)
		{
			_delay_loop_2(0);
			_delay_loop_2(0);
			_delay_loop_2(0);
		}

	}

void Clockwise_Tick(){
	//Actions
	switch(step_state){
		case WAIT:
		break;
		case A:
		PORTA = 0x01;
		break;
		case AB:
		PORTA = 0x03;
		break;
		case B:
		PORTA = 0x02;
		break;
		case BC:
		PORTA = 0x06;
		break;
		case C:
		PORTA = 0x04;
		break;
		case CD:
		PORTA = 0x0C;
		break;
		case D:
		PORTA = 0x08;
		break;
		case DA:
		PORTA = 0x09;
		break;
		default:
		break;
	}
	//Transitions
	switch(step_state){
		case WAIT:
		step_state = A;
		break;
		case A:
		step_state = AB;
		break;
		case AB:
		step_state = B;
		break;
		case B:
		step_state = BC;
		break;
		case BC:
		step_state = C;
		break;
		case C:
		step_state = CD;
		break;
		case CD:
		step_state = D;
		break;
		case D:
		step_state = DA;
		break;
		case DA:
		step_state = A;
		break;
		default:
		break;
	}
}

void CounterClockwise_Tick(){
	//Actions
	switch(step_state){
		case WAIT:
		break;
		case A:
		PORTA = 0x01;
		break;
		case AB:
		PORTA = 0x03;
		break;
		case B:
		PORTA = 0x02;
		break;
		case BC:
		PORTA = 0x06;
		break;
		case C:
		PORTA = 0x04;
		break;
		case CD:
		PORTA = 0x0C;
		break;
		case D:
		PORTA = 0x08;
		break;
		case DA:
		PORTA = 0x09;
		break;
		default:
		break;
	}
	//Transitions
	switch(step_state){
		case WAIT:
		step_state = DA;
		break;
		case A:
		step_state = DA;
		break;
		case AB:
		step_state = A;
		break;
		case B:
		step_state = AB;
		break;
		case BC:
		step_state = B;
		break;
		case C:
		step_state = BC;
		break;
		case CD:
		step_state = C;
		break;
		case D:
		step_state = CD;
		break;
		case DA:
		step_state = D;
		break;
		default:
		break;
	}
}

void ServoTick(){
	//Actions
	switch(servo_state){
		case UNLOCK:
			if(!unlocked)
			{
					DDRD|=(1<<PD4)|(1<<PD5);
					TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1 << WGM11);
					TCCR1B |= (1<<WGM13)  | (1<<WGM12)  | (1<<CS11)   | (1 << CS10);
					ICR1 = 4999;

				OCR1A = 160;
				Wait();
				OCR1A = 97;
				Wait();
				//servo_state = OPEN;
			}
			unlocked = 1;
			break;

		case LOCK:
		if(unlocked && !unlock)
		{
			OCR1A = 97;
			Wait();
		}
		unlocked =1;
			break;
		default:
			servo_state = LOCK;
			break;
	}
	switch(servo_state){
			case UNLOCK:
				if(unlocked )
				{
					servo_state = LOCK;
				}
				break;
			case LOCK:
				if(unlock)
				{
					servo_state = UNLOCK;
				}
				break;
			default:
				servo_state = LOCK;
				//OCR1A = servo;
				break;
		}
}

void ClockwiseTask()
{
	for(;;)
	{
		if(logged_in == '1')
		{
			
			if(GetBit(PINC, 0) && (!GetBit(PINC,1)))
				Clockwise_Tick();

			vTaskDelay(3);
		}
	}
}

void CounterClockwiseTask()
{
	for(;;)
	{
		if(logged_in == '1')
		{
			
			if(GetBit(PINC, 1) && (!GetBit(PINC,0)) )
				CounterClockwise_Tick();
		}
		vTaskDelay(3);
	}
}




void ServoTask()
{
	for(;;)
	{
		if(USART_HasReceived(1))
		{
			logged_in = USART_Receive(1);
			USART_Flush(1);
			//USART_Flush(0);
			PORTB |= 0x01;
			if(logged_in == '0')
			{
				PORTB &= 0xFE;
			}
		}
		if(USART_HasReceived(0))
		{
			bluetooth = USART_Receive(0);
			//PORTB |= 0x01;
			PORTB |= 0x02;
			USART_Flush(0);
			if(bluetooth == '1')
			{
				
				
				DDRD|=(1<<PD4)|(1<<PD5);
				TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1 << WGM11);
				TCCR1B |= (1<<WGM13)  | (1<<WGM12)  | (1<<CS11)   | (1 << CS10);
				ICR1 = 4999;
			
				OCR1A = 200;
				Wait();
			
				//TCCR1A = 0;
				//TCCR1B = 0;
				//ICR1 = 0;
			}
			else if (bluetooth == '2')
			{
				DDRD|=(1<<PD4)|(1<<PD5);
				TCCR1A |= (1<<COM1A1) | (1<<COM1B1) | (1 << WGM11);
				TCCR1B |= (1<<WGM13)  | (1<<WGM12)  | (1<<CS11)   | (1 << CS10);
				ICR1 = 4999;
			
				OCR1A=97;
				Wait();
			
				//TCCR1A = 0;
				//TCCR1B = 0;
				//ICR1 = 0;
			}
			PORTB &= 0xFE;
		}
			//unlock = 1;
		//if(!unlocked)
		//{
			//
		//}
			////ServoTick();
		//if (unlocked && !unlock)
		//{
			////unlocked = 0;
			////servo_state = LOCK;
			//ServoTick();
		//}
		vTaskDelay(100);	
	}
}


void StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(ClockwiseTask,			(signed portCHAR *)"ClockwiseTask",			configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(CounterClockwiseTask,	(signed portCHAR *)"CounterClockwiseTask",	configMINIMAL_STACK_SIZE, NULL, Priority, NULL);
	xTaskCreate(ServoTask,				(signed portCHAR *)"ServoTask",				configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	//xTaskCreate(TVTask,					(signed portCHAR *)"TVTask",			configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	//xTaskCreate(KeypadTask,				(signed portCHAR *)"KeypadTask",			configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void servoInit()
{
	servo_state = LOCK;
}

void stepInit(){
	step_state = WAIT;
}


int main(void)
{
	// slave
		DDRA = 0xFF; //PORTA=0xFF; //pins 6/7 for stepper control
		//DDRD = 0xFF; //output except for USART shit
		DDRC = 0xFF;
		DDRB = 0xFF;
		//PORTC = 0x00; PORTB = 0x00;
		DDRD = 0xFF;
		
	//master
	/*
		DDRA = 0x00;
		DDRB = 0x00;
		DDRC = 0x00;
		DDRD = 0xFF; //usart shit again
	*/
	
	PORTB = 0xE0;
	delay_ms(2000);
	initUSART(0);
	initUSART(1);
	USART_Flush(1);
	USART_Flush(0);
	
	//while(1)
	//{
	//
      //OCR1A=97;   //0 degree
      //Wait();
	  //OCR1A = 222;
	  //Wait();
//
	//}
	
	//OCR1A = 97;
	//Wait();

	//while(1)
	//{
		//
		////if(GetBit(PINB,0))
		////{
////
			////OCR1A = 222;
			////Wait();
		////}
		////if (GetBit(PINB, 1))
		////{
			////OCR1A=97;
			////Wait();
		////}
	//}
	//Init State
	stepInit();
	servoInit();
	//bluetoothInit();
	//keypadInit();
	PORTB = 0x00;
	
	//Start Tasks
	StartSecPulse(1);
	//RunSchedular
	vTaskStartScheduler();
	
	return 0;
}