#include <termios.h>

#include <stdlib.h>
#include <stdio.h>

#include <bcm2835.h>

#define PIN_FAIL RPI_GPIO_P1_12
#define PIN_SUCCESS RPI_GPIO_P1_16
int connect_to_serial() {
	if (!bcm2835_init()) {
		return 1;
	}
	
	bcm2835_gpio_fsel(PIN_SUCCESS, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PIN_FAIL,BCM2835_GPIO_FSEL_OUTP);
}

void has_valid_card() {
	bcm2835_gpio_write(PIN_SUCCESS, HIGH);
	delay(500);
	bcm2835_gpio_write(PIN_SUCCESS,LOW);
}
void has_invalid_card() {
	bcm2835_gpio_write(PIN_FAIL, HIGH);
	delay(500);
	bcm2835_gpio_write(PIN_FAIL,LOW);
}
