/*
===============================================================================
 Name        : main.c
 Author      : $(author)
 Version     :
 Copyright   : $(copyright)
 Description : main definition
===============================================================================
*/

#ifdef __USE_CMSIS
#include "LPC13Uxx.h"
#endif

#include <cr_section_macros.h>

#include "i2c_master.h"
#include "st7032i.h"

// LEDのポート番号と、ピン番号を定義
#define LED_PORT 0
#define LED_PIN 7

static const uint8_t LED_ADDR = 0xE2;
static const uint8_t TRM_ADDR = 0x90;
static const uint8_t LCD_ADDR = 0x7C;

void gpio_init() {
	// GPIOドメインの電源を入れる
	LPC_SYSCON->SYSAHBCLKCTRL |= (1<<6);
}

/**
 * GPIOのピンの入出力方向を設定する。
 */
void gpio_set_dir(int port, int pin, int do_output) {
	if (do_output) {
		LPC_GPIO->DIR[port] |= (1 << pin);
	} else {
		LPC_GPIO->DIR[port] &= ~(1 << pin);
	}
}

/**
 * GPIOの特定ピンの出力内容を設定する。
 */
void gpio_set_output(int port, int pin, int enable) {
    if (enable) {
    	LPC_GPIO->SET[port] = (1 << pin);
    } else {
    	LPC_GPIO->CLR[port] = (1 << pin);
    }
}

// 割り込みカウンタ。1秒ごとにゼロリセット
volatile unsigned long systick_count = 0;
// 1/10秒ごとにカウントするカウンタ
volatile unsigned long sec_count = 0;

/**
 * SysTick用割り込みハンドラ
 */
void SysTick_Handler(void) {
	systick_count++;
	if (100 <= systick_count) {
		systick_count = 0;
		sec_count++;
		if (10000 < sec_count) {
			sec_count = 0;
		}
	}
}

static inline uint8_t *format_digit(uint8_t *buf, unsigned int length, unsigned int num) {
  unsigned int factor = 1;
  for (uint8_t i = 1; i < length; i++) {
    factor = factor * 10;
  }
  for (uint8_t i = 0; i < length; i++) {
    unsigned int val = num / factor;
    buf[i] = '0' + val;
    num %= factor;
    factor /= 10;
  }
  return buf;
}

/**
 * マイクロ秒単位で指定時間待つ
 */
void wait(uint8_t ms) {
	for (uint8_t i = 0; i < ms; i++) {
		unsigned long last_systick = systick_count;
		while (last_systick == systick_count) ;
	}
}

int main(void) {
	// クロック設定の更新
	SystemCoreClockUpdate();
	// 1msごとにSysTick割り込みを設定
	SysTick_Config(SystemCoreClock / 1000);

	// I2Cマスタライブラリを400kHzで初期化
	I2CMasterInit(400000);

	// GPIOの初期化
	gpio_init();
	// LED用のGPIOピンを出力に設定
	gpio_set_dir(LED_PORT, LED_PIN, 1);

	// I2C用の送受信バッファ
	uint8_t i2cbuf[5];
	// LEDモジュールを初期化する。
	i2cbuf[0] = 0x76; // 表示のリセット
	I2CMasterTX(LED_ADDR, i2cbuf, 1);

	// 温度センサを16bitモードに設定する。
	i2cbuf[0] = 0x03; // レジスタアドレス
	i2cbuf[1] = 0x80; // 設定値
	I2CMasterTX(TRM_ADDR, i2cbuf, 2);

	// LCDモジュールを初期化する
	st7032i_init(LCD_ADDR, wait);
	st7023i_clear_display(LCD_ADDR);
	st7032i_display(LCD_ADDR, 6, 0, "\xF2" "C");
	st7032i_display(LCD_ADDR, 0, 1, "GxP Temp");

	// LEDの次の出力状態を保持する変数
	int led_on = 0;
	// 7セグLEDの表示値を保持する変数
	unsigned long led_value = -1;
    // Enter an infinite loop
    while(1) {
    	// 点灯すべきかどうか計算
    	int next_led = (sec_count % 10) < 5;
    	if (led_on != next_led) {
            // led_onに合わせてLEDを点灯したり消灯したりする
        	gpio_set_output(LED_PORT, LED_PIN, next_led);
        	// LEDの出力状態を更新する
    		led_on = next_led;
    	}
    	if (led_value != sec_count) {
    		// 表示値を更新
    		led_value = sec_count;
    		// 温度を取得する。
    		I2CMasterRx(TRM_ADDR, i2cbuf, 4);
    		// 16bit読み出し値に変換
    		int termo_value = (i2cbuf[0] << 8) | i2cbuf[1];
    		// 計測単位が0.0078度Cごとなので、1/100度で表示できるよう係数をかける
    		termo_value = termo_value * 0.78;
    		// 4桁で数値を文字列に変換してバッファに格納
    		format_digit(i2cbuf, 4, termo_value);
    		i2cbuf[4] = 0x00; // nullターミネータを設定
    		// 4文字をLEDに書き込む
    		I2CMasterTX(LED_ADDR, i2cbuf, 4);
    		// 値をLCDに書き込む
    		char lcdbuf[10];
    		lcdbuf[0] = i2cbuf[0];
    		lcdbuf[1] = i2cbuf[1];
    		lcdbuf[2] = '.';
    		lcdbuf[3] = i2cbuf[2];
    		lcdbuf[4] = i2cbuf[3];
    		lcdbuf[5] = 0x00;
    		st7032i_display(LCD_ADDR, 1, 0, lcdbuf);
    	}
    }
    return 0;
}
