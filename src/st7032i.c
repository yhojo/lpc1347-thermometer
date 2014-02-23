
#include "LPC13Uxx.h"
#include "st7032i.h"

#include "i2c_master.h"

static const int LCD_WIDTH = 8;
static const int LCD_HEIGHT = 2;

static void (*mywait)(uint8_t ms);

void st7032i_init(uint8_t slave_addr, void (*wait)(uint8_t ms)) {
	mywait = wait;
	mywait(20);
	// LCDの初期化
	st7032i_send_command(slave_addr, 0b00111000); // function set off
	mywait(1);
	st7032i_send_command(slave_addr, 0b00111001); // function set on
	mywait(1);
	st7032i_send_command(slave_addr, 0b00010100); // setup bias and OSC frequency
	mywait(1);
	st7032i_send_command(slave_addr, 0b01110000); // set contrast function
	mywait(1);
	st7032i_send_command(slave_addr, 0b01010110); // set contrast (Power/Icon/Contrast)
	mywait(1);
	st7032i_send_command(slave_addr, 0b01101100); // setup follower control
	mywait(200);
	st7032i_send_command(slave_addr, 0b00111000); // function set off
	mywait(1);
	st7032i_send_command(slave_addr, 0b00001100); // Display on
	mywait(1);
}

int8_t st7032i_send_command(uint8_t slave_addr, uint8_t command) {
	uint8_t buf[2];
	buf[0] = 0x00;
	buf[1] = command;
	return I2CMasterTX(slave_addr, buf, 2);
}

int8_t st7032i_send_data(uint8_t slave_addr, uint8_t data) {
	uint8_t buf[2];
	buf[0] = 0x40;
	buf[1] = data;
	return I2CMasterTX(slave_addr, buf, 2);
}

void st7023i_clear_display(uint8_t slave_addr) {
	st7032i_send_command(slave_addr, 0b00000001); // clear display
	mywait(1);
}

void st7032i_display(uint8_t slave_addr, uint8_t x, uint8_t y, char *str) {
	uint8_t start_addr = y * 0x40 + x;
	st7032i_send_command(slave_addr, 0b10000000 | start_addr);
	for (uint8_t i = 0; i < (LCD_WIDTH - x) && str[i]; i++) {
		st7032i_send_data(slave_addr, str[i]);
	}
}
