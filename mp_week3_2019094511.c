#include <avr/io.h>
void my_delay(unsigned long x) {
	while(x--);
}

int main(void) {
	char i;
	DDRA = 0xFF;
	while(1) {
		for(i=0; i<8; i++) {
			PORTA=0x01<<i;
			my_delay(100000);
		}
		for(i=0; i<8; i++) {
			PORTA=0x80>>i;
			my_delay(100000);
		}
	}
	return 0;
}
