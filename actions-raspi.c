#include <termios.h>

#include <stdlib.h>
#include <stdio.h>

#include <bcm2835.h>

#define PIN_FAIL RPI_GPIO_P1_12
#define PIN_LED_SUCCESS RPI_GPIO_P1_16
#define PIN_DOOR_SUCCESS RPI_GPIO_P1_22
#define PIN_DOOR_OPEN RPI_GPIO_P1_24
int connect_to_serial() {
	if (!bcm2835_init()) {
		return 1;
	}
	
	bcm2835_gpio_fsel(PIN_LED_SUCCESS, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PIN_DOOR_SUCCESS, BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PIN_FAIL,BCM2835_GPIO_FSEL_OUTP);
	bcm2835_gpio_fsel(PIN_DOOR_OPEN,BCM2835_GPIO_FSEL_INPT);
}

void has_valid_card() {
	bcm2835_gpio_write(PIN_LED_SUCCESS, HIGH);
	bcm2835_gpio_write(PIN_DOOR_SUCCESS,HIGH);
	delay(5000);
	delay(5000);
	bcm2835_gpio_write(PIN_LED_SUCCESS,LOW);
	bcm2835_gpio_write(PIN_DOOR_SUCCESS,LOW);
}
void has_invalid_card() {
	bcm2835_gpio_write(PIN_FAIL, HIGH);
	delay(500);
	bcm2835_gpio_write(PIN_FAIL,LOW);
}


uint8_t read_door_open() {
        return bcm2835_gpio_lev(PIN_DOOR_OPEN);
}       

