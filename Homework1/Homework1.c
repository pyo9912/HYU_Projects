#include <avr/io.h>

int main(void){
	DDRA = 0xFF;
	PORTA = 0x7F;

	return 0;
}
