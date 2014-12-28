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
/*  FreeRTOS include files  */
#include "FreeRTOS.h"
#include "task.h"
#include "croutine.h"
#include "keypad.h"
#include "lcd.h"
#include "usart_ATmega1284.h"

//int quarterPhase = (90 / 5.625) * 64;
int phase = 0;
unsigned char flag = 0; //Used to go between keypad and motor


int num_steps = 0; //steps to take via user input
unsigned char rotation[4] = { '3', '6', '0', '#' }; //used to get total user input

unsigned char temp = 0;
unsigned char input = '\0'; //used to get individual user input

unsigned char bluetooth = 0;


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
///////////////////////////////                     VARIABLES                      ///////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
const unsigned char* introMessage =  "Entertainment    Control";
const unsigned char* introMessage2 = "Please enter      password";
const unsigned char* introMessage3 = "matching the key";
const unsigned char* passwordConfirm1 = "Password Valid";
const unsigned char* passwordConfirm2 = "Logged In";
const unsigned char* invalid_pass = "Invalid Pass,   new key";
//const unsigned char* current_key_message = "Current Key: ";
const unsigned char* enter_pass_message = "Enter Matching      Password";
const unsigned char* access_msg0 = "0: volume/input control only";
const unsigned char* access_msg1 = "1: 0's controls + lock";
const unsigned char* inputa = "TV Input changed     to HDMI 1";
const unsigned char* inputb = "TV Input changed     to HDMI 2";
const unsigned char* inputc = "TV Input changed     to Antenna";
const unsigned char* inputd = "TV Input changed     to RGB";

const unsigned char* curr_acc_msg0 = "0: volume/input control only";
const unsigned char* curr_acc_msg1 = "1: vol/input and lock control";

unsigned char* current_key = "    ";

unsigned char masterPassword[5] = { ' ', ' ', ' ', ' ', '\0'};
//unsigned char* masterPassword = "";
const unsigned char* menuMessage = "# to Logout     0 to see access";
const unsigned char* menuMessage1 = "A,B,C,D For TV  Input Change";
int passwordLength = 4;
unsigned char usartInput = 0x00;
unsigned char usartOutput = 0x00;

unsigned char passwords[4][5] = {{'4','5','6','7','\0'},
						{'A','B','C','D','\0'},
						{'6','7','6','7','\0'},
						{'9','8','7','6','\0'}};
//passwords[0] = {'4','5','6','7','\0'};
//passwords[1] = {'8','6','7','5','\0'};
//passwords[2] = {'1','5','4','7','\0'};
//passwords[3] = {'A','B','C','D','\0'};

unsigned char keys[4][8] =  {{'K', 'e', 'y', ':', ' ', '1'},
							 {'K', 'e', 'y', ':', ' ', '2'},
							 {'K', 'e', 'y', ':', ' ', '3'},
							 {'K', 'e', 'y', ':', ' ', '4'}};
//keys[0] = "Key = 1";
//keys[1] = "Key = 2";
//keys[2] = "Key = 3";
//keys[3] = "Key = 4";

unsigned char accesses[5] = {'1','0','1','0','\0'};


char logged_in = 0x00;
int curr_rand = 0;
unsigned char curr_access = '1';
unsigned char* curr_pass = "4567";
char* try_pass = "    ";




unsigned char admin_pass [5] = {'D','D','D','D','\0'}; 

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////////                     Flags                      /////////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
unsigned char lockingFlag = 0x00;
unsigned char lockStatus = 0x00; //0x00 for close, 0x01 for open

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////                     ENUMERATIONS                      /////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

enum LeaderState {l_INIT, l_send} leader_state;

bool getInput()
{
	input = GetKeypadKey();
	if(input == '\0')
	return false;
	else
	return true;
}

unsigned char listenForInput()
{
	if(!getInput())
	{
		while(!getInput());
	}
	return input;
}

void setPassword()
{
	while(!logged_in)
	{
		//LCD_DisplayString(1, current_key_message);
		//delay_ms(2000);

		unsigned char masterPasswordView[5] = { ' ', ' ', ' ', ' ', '\0'};
	
		for(int i = 0; i < 4; ++i)
		{
			
			LCD_DisplayString(1, enter_pass_message);
			delay_ms(1000);
			LCD_DisplayString(1, keys[curr_rand]);
			delay_ms(1000);
			
			//masterPassword[i] = listenForInput();
			try_pass[i] = listenForInput();
			//masterPasswordView[i] = try_pass[i];
			
			//LCD_DisplayString(1, keys[curr_rand]);
			//delay_ms(1000);
			//LCD_DisplayString(1, enter_pass_message);
			//delay_ms(1000);
			
			//input = '\0';
			//delay_ms(1000);
			//masterPasswordView[i] = '*';
			//LCD_DisplayString(1, masterPasswordView);
		
		}
	
		LCD_DisplayString(1, try_pass);
		delay_ms(3000);
		LCD_DisplayString(1, curr_pass);
		delay_ms(3000);
	
		//strcmp()
	
		char valid = 1;
		for(int i = 0; i < 4; ++i)
		{
			if(try_pass[i] != curr_pass[i])
			{
				valid = 0;
			}
		}
		if(valid)
		{
			logged_in = 1;
			PORTB |= 0x01;
			
			if (curr_access == '1')
			{
				PORTB |= 0x04;
			}
			else
			{
				PORTB &= 0xFD;
			}
			
			USART_Flush(0);

			LCD_DisplayString(1, passwordConfirm1);
			delay_ms(3000);

			//LCD_DisplayString(1, masterPassword);
			//delay_ms(3000);
			LCD_DisplayString(1,passwordConfirm2);
			delay_ms(3000);
		}
		else
		{
			LCD_DisplayString(1, invalid_pass);
			delay_ms(3000);
		
			curr_rand += 3;
			if(curr_rand >= 4)
				curr_rand %= 4;
			//masterPassword = passwords[curr_rand];
			curr_pass = passwords[curr_rand];
			curr_access = accesses[curr_rand];
		
			setPassword();
		}
			//if(listenForInput() == '#')
		//{
			//LCD_DisplayString(1, "Password Set");
			//delay_ms(2000);
		//}
		//else if(listenForInput() == '*')
		//{
			//LCD_DisplayString(1, "Password Reset");
			//delay_ms(2000);
			//setPassword();
		//}
	}
}

void menuChoose()
{
	LCD_DisplayString(1, menuMessage);
	delay_ms(2000);
	LCD_DisplayString(1, menuMessage1);
	delay_ms(2000);
	
	if(listenForInput() == 'A')
	{
		//LCD_DisplayString(1, "Enter new Password");
		//delay_ms(3000);
		//setPassword();
				LCD_DisplayString(1, inputa);
				delay_ms(2000);
	}
	else if(listenForInput() == 'B')
	{
				LCD_DisplayString(1, inputb);
				delay_ms(2000);
		//lockingFlag = 0x01;
		//LCD_DisplayString(1, "Now Locking...");
		//delay_ms(3000);
	}
	else if(listenForInput() == 'C')
	{
				LCD_DisplayString(1, inputc);
				delay_ms(2000);
		//lockingFlag = 0x00;
		//LCD_DisplayString(1, "This is total garbage");
		//delay_ms(10000);
		//if(lockingFlag == 0x00)
		//LCD_DisplayString(1, "Dirty Dog");
		//delay_ms(5000);
	}
	else if(listenForInput() == 'D')
	{
		LCD_DisplayString(1, inputd);
		delay_ms(2000);
	}
	else if(listenForInput() == '#')
	{
		logged_in = 0;
		
		while(!USART_IsSendReady(0)){}
		USART_Send('0', 0);
		PORTB |= 0x04;
		while(!USART_HasTransmitted(0)){}
		USART_Flush(0);
		
		bluetooth = 0x00;
		PORTB &= 0xFE;
		PORTB &= 0xFB;
		
		update_credsTick();
		setPassword();
	}
	else if(listenForInput() == '0')
	{
		LCD_DisplayString(1, access_msg0);
		delay_ms(2000);
		LCD_DisplayString(1, access_msg1);
		delay_ms(2000);
		
		if(curr_access == '1')
		{
			LCD_DisplayString(1,curr_acc_msg1);
			delay_ms(2000);
		}
		else
		{
			LCD_DisplayString(1, curr_acc_msg0);
			delay_ms(2000);
		}
	}
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
/////////////////////////////                     TICK FUNCTIONS                      ////////////////////////////////
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void SM_Menu_Tick(){
	if(!logged_in)
	{
		//LCD_DisplayString(1, menuMessage);
		//delay_ms(3000);
		//update_credsTick();
		setPassword();
	}
	menuChoose();
}

void Leader_Tick(){
	//Actions
	switch(leader_state)
	{
		case l_INIT:
		break;
		
		case l_send:
			if(USART_HasReceived(0) && (bluetooth == 0x00))
			{
				bluetooth = USART_Receive(0);
				PORTB |= 0x02;
		
				USART_Flush(0);
				
				if (!logged_in && bluetooth != 0x00 )
				{
					PORTB |= 0x08;
					bluetooth = 0x00;
					//alarm !?
				}
				else if (bluetooth != 0x00 && curr_access != '1')
				{
					//alarm
					PORTB |= 0x08;
					bluetooth = 0x00;
				}
			}
			if(logged_in)
			{
				//USART_Flush(0);
				while(!USART_IsSendReady(0)){}
				USART_Send('1', 0);
				PORTB |= 0x04;
				PORTB &= 0xF7;
				while(!USART_HasTransmitted(0)){}
				USART_Flush(0);
			}
		
		
		if(logged_in && (bluetooth != 0x00) && (curr_access == '1'))
		{
			//USART_Flush(1);
			while(!USART_IsSendReady(1)){}
			USART_Send(bluetooth, 1);
			PORTB |= 0x04;
			while(!USART_HasTransmitted(1)){}
			USART_Flush(1);
			
			bluetooth = 0x00;
			PORTB &= 0xF9;
		}
		//PORTB = usartInput;
		//usartInput = (usartInput == 0x01) ? 0x00 : 0x01;
		break;
		
		default:
		break;
	}
	
	//Transitions
	switch(leader_state){
		case l_INIT:
		//USART_Flush(0);
		//if (USART_IsSendReady(0))
		//{
			leader_state = l_send;
		//}
		//else
		//leader_state = l_INIT;
		break;
		
		case l_send:
		//USART_Flush(0);
		//while(!USART_HasTransmitted(0)){}
		//lockingFlag = 0x01;
		//leader_state = l_INIT;
		break;
		
		default:
		leader_state = l_INIT;
		break;
	}
}


void LeaderSecTask()
{

	for(;;)
	{
		//if(lockingFlag == 0x01)
		//{
			////PORTB = 0x02;
			//Leader_Tick();
			//vTaskDelay(100);
		//}
		//else
		//{
			//USART_Flush(0);
		//}
		//PORTB = 0x00;
		Leader_Tick();
		vTaskDelay(1000);
	}
	
}

void SM_Menu()
{
	for(;;)
	{
		SM_Menu_Tick();
		vTaskDelay(15);
	}
}

void LeaderInit()
{
	leader_state = l_send;
}

void update_credsTick()
{
	//curr_rand = rand();
	curr_rand += 3;
	if(curr_rand >= 4)
		curr_rand %= 4;
	//masterPassword = passwords[curr_rand];
	curr_pass = passwords[curr_rand];
	curr_access = accesses[curr_rand];
	//PORTB = 0xFF;
	
}

update_credsTask()
{
	for(;;)
	{
		if(!logged_in)
		{
			update_credsTick();
			vTaskDelay(15000);
		}
	}
}


void SM1_StartSecPulse(unsigned portBASE_TYPE Priority)
{
	xTaskCreate(update_credsTask, (signed portCHAR *)"update_credsTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(SM_Menu, (signed portCHAR *)"SM_Menu", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
	xTaskCreate(LeaderSecTask, (signed portCHAR *)"LeaderSecTask", configMINIMAL_STACK_SIZE, NULL, Priority, NULL );
}

void showMenu()
{
	LCD_DisplayString(1, menuMessage);
	delay_ms(3000);
	menuChoose();
}

void setUpMenu()
{
	LCD_DisplayString(1, introMessage);
	delay_ms(3000);
	LCD_DisplayString(1, introMessage2);
	delay_ms(3000);
	LCD_DisplayString(1, introMessage3);
	delay_ms(3000);
	setPassword();
}


int main(void)
{
	DDRA = 0xF0; PORTA = 0x0F;
	DDRB = 0xFF; PORTB = 0x00;
	DDRC = 0xFF; PORTC = 0x00;
	DDRD = 0xFF; PORTD = 0x00;
	
	initUSART(0);
	initUSART(1);
	
	//srand();
	
	USART_Flush(0);
	USART_Flush(1);	
	//while(1)
	//{
		//if(USART_HasReceived(0))
		//{
			//bluetooth = USART_Receive(0);
			//PORTB = 0xFF;
		//}
	//}
	
	//update_creds();
	LCD_init();
	setUpMenu();
	LeaderInit();


	SM1_StartSecPulse(1);
	vTaskStartScheduler();

}