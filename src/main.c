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

// LEDのポート番号と、ピン番号を定義
#define LED_PORT 0
#define LED_PIN 7

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

/**
 * SysTick用割り込みハンドラ
 */
void SysTick_Handler(void) {
	systick_count++;
	if (1000 <= systick_count) {
		systick_count = 0;
	}
}

int main(void) {
	// クロック設定の更新
	SystemCoreClockUpdate();
	// 1msごとにSysTick割り込みを設定
	SysTick_Config(SystemCoreClock / 1000);

	// GPIOの初期化
	gpio_init();
	// LED用のGPIOピンを出力に設定
	gpio_set_dir(LED_PORT, LED_PIN, 1);

	// LEDの次の出力状態を保持する変数
	int led_on = 0;
    // Enter an infinite loop
    while(1) {
    	// 点灯すべきかどうか計算
    	int next_led = systick_count < 500;
    	if (led_on != next_led) {
            // led_onに合わせてLEDを点灯したり消灯したりする
        	gpio_set_output(LED_PORT, LED_PIN, systick_count < 500);
        	// LEDの出力状態を更新する
    		led_on = next_led;
    	}
    }
    return 0;
}
