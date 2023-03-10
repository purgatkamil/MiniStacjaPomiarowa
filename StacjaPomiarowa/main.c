#define F_CPU 16000000

#include <avr/io.h>
#include <stdio.h>
#include <util/delay.h>

void ADC_Init(void)
{
	ADMUX = (1 << REFS0);
	ADCSRA = (1 << ADEN) | (1 << ADPS2) | (1 << ADPS1) | (1 << ADPS0);
}

long int getADC(char channel)
{
	ADMUX = 0b00000000;
	ADMUX = (1 << REFS0);
	long int W = 0;
	ADMUX |= channel;
	ADCSRA |= (1 << ADSC);
	while(ADCSRA & (1 << ADIF));
	ADCSRA |= (1 << ADIF);
	W = ADCL;
	W |= (ADCH << 8);
	return W;
}
void SendChar(unsigned char dana)
{
	DDRD = 0b00111111;
	PORTD |= 0b00110000; //RS = 1 i E = 1
	PORTD = ((PORTD & 0b11110000) | ((dana>>4) & 0b00001111));
	PORTD &= 0b11101111; //E = 0
	_delay_loop_2(5000);
	PORTD |= 0b00010000; //E = 1
	PORTD = ((PORTD & 0b11110000) | (dana & 0b00001111));
	PORTD &= 0b11101111; //E = 0
	_delay_loop_2(5000);
	PORTD |= 0b00010000; //E = 1
}
void SendCommand(unsigned char dana)
{
	DDRD = 0b00111111;
	PORTD &= 0b11011111; //RS = 0
	PORTD |= 0b00010000; //E = 1
	PORTD = ((PORTD & 0b11110000) | ((dana>>4) & 0b00001111));
	PORTD &= 0b11101111; //E = 0
	_delay_loop_2(25000);
	PORTD |= 0b00010000; //E = 1
	PORTD = ((PORTD & 0b11110000) | (dana & 0b00001111));
	PORTD &= 0b11101111; //E = 0
	_delay_loop_2(25000);
	PORTD |= 0b00010000; //E = 1
}

void LCD_Init(void)
{
	SendCommand(0x33);
	SendCommand(0x32);
	SendCommand(0x28);
	SendCommand(0x08);
	SendCommand(0x01);
	SendCommand(0x06);
	SendCommand(0x0F);
}

void Button_Init(){
	DDRB &= ~(1 << PB0);
	PORTB |= (1 << PB0);
}

void TemperatureMeasurement(){
	char temperature[20];
	long int TempResult = getADC(0);
	TempResult = (TempResult * 50000)/1024;
	ltoa(TempResult, temperature, 10);
	SendChar(temperature[0]);
	SendChar(temperature[1]);
	SendChar(',');
	SendChar(temperature[2]);
	
}

void VoltageMeasurement(){
	char voltage[20];
	long int VoltResult = getADC(1);
	VoltResult = (VoltResult * 50)/1024;
	ltoa(VoltResult, voltage, 10);
	if (VoltResult >= 10){
		SendChar(voltage[0]);
		SendChar(',');
		SendChar(voltage[1]);
	}
	else{
		SendChar('0');
		SendChar(',');
		SendChar(voltage[0]);
	}	
}

void Display(int *mode){
	
	switch (*mode){
		
		case 0:
			VoltageMeasurement();
			break;
		
		case 1:
			TemperatureMeasurement();
			break;
		
	}
	
}

void ChangeDisplayMode(int *mode, int *change){
	
	if(((PINB & 0b00000001) == 0) && (*change == 0)){
		*change = 1;
		if(*mode == 1)
			*mode = 0;
		else
			*mode = 1;
	}
	if((PINB & 0b00000001) != 0) *change = 0;
}

int main(void)
{
	int Change = 0;
	int DisplayMode = 1;
	LCD_Init();
	ADC_Init();
	Button_Init();

    while (1) {	
		ChangeDisplayMode(&DisplayMode, &Change);
		SendCommand(0x01);				//Clearing LCD
		Display(&DisplayMode);
		_delay_ms(100);		
		
    }
}
