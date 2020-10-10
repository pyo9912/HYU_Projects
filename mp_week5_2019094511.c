#include <avr/io.h>

int main(void) {
	
	unsigned char input_data_rot_button;
	DDRA = 0x00;
	DDRB = 0xFF;

	while(1) {
		input_data_rot_button = PINA & 0xFF;
		switch(input_data_rot_button) {
			case 0:
				PORTB = 0b10000000;
				break;
			case 1:
				PORTB = 0b01000000;
				break;
			case 2:
				PORTB = 0b00100000;
				break;
			case 3:
				PORTB = 0b00010000;
				break;
			case 4:
				PORTB = 0b00001000;
				break;
			case 5:
				PORTB = 0b00000100;
				break;
			case 6:
				PORTB = 0b00000010;
				break;
			case 7:
				PORTB = 0b00000001;
				break;
			default:
				PORTB = 0b00000000;
				break;
		}
	}

}
