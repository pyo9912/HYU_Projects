#include <avr/io.h>

const char Font[17] = {0x3F, 0x06, 0x5B, 0x4F, 0x66, 0x6D,  //0, 1, 2, 3, 4, 5
		0x7D, 0x07, 0x7F, 0x6F, 0x77, 0x7C,  //6, 7, 8, A, B
		0x39, 0x5E, 0x79, 0x71, 0x00};  //C, D, E, F, NULL

void delay(unsigned long x) {
	while(x--);
}

int main(void) {
	char i, j;
	unsigned char digit0, digit1, digit2;
	DDRB=0xFF;
	DDRG=0xFF;

	PORTB=0x00;
	PORTG=0xFF;

	while(1) {
		for(i=0; i<1000; i++) {
			digit0 = i/100;
			digit1 = i%100/10;
			digit2 = i%10;

			for(j=0; j<100; j++) {
				PORTG |= 1<<2;
				PORTB = Font[digit0];
				delay(100);
				PORTG &= ~(1<<2);

				PORTG |= 1<<1;
				PORTB = Font[digit1];
				delay(100);
				PORTG &= ~(1<<2);

				PORTG |= 1<<0;
				PORTB = Font[digit2];
				delay(100);
				PORTG &= ~(1<<0);
			}
		}
	}
	return 0;
}
