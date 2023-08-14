#include <avr/io.h>

#define LCD_CTRL PORTG
#define LCD_DATA PORTA
#define LCD_RS 0x01
#define LCD_RW 0x02
#define LCD_E 0x04

void delay(unsigned long x) {
	while(x--);
}

void enablePulse(void) {
	LCD_CTRL |= LCD_E;
	asm("NOP");
	LCD_CTRL &= ~LCD_E;
}

void sendLCDCommand(unsigned char command) {
	LCD_CTRL &= ~LCD_RS;
	LCD_DATA = command;
	enablePulse();
	delay(20);
}

void sendLCDData(unsigned char data) {
	LCD_CTRL |= LCD_RS;
	LCD_DATA = data;
	enablePulse();
	delay(1000);
}

void initLCD(void) {
	delay(20000);
	LCD_CTRL &= ~LCD_RW;
	sendLCDCommand(0x38);
	sendLCDCommand(0x0C);
	sendLCDCommand(0x01);
	delay(1000);
}

void locate(int x, int y) {
	unsigned char ramAddr;
	if(x==0)
		ramAddr = 0x80 + y;
	else
		ramAddr = 0xC0 + y;
	sendLCDCommand(ramAddr);
}

int main(void) {
	DDRA = 0xFF;
	DDRG = 0xFF;
	PORTA = 0x00;
	PORTG = 0x00;

	initLCD();

	sendLCDData('1');
	sendLCDData('2');
	sendLCDData('3');
	sendLCDData('4');

	locate(1,0);

	while(1);
}
