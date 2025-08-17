/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/led.h>
#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>

#include "kc.h"
#include "snake.h"

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define BTN_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_keys)

void breath_thread_func()
{
	const struct device *const led = DEVICE_DT_GET(LED_NODE);
	if ( ! device_is_ready(led)) {
		printk("[bt] LED device not ready\n");
		return;
	}
	// n=15; [int(.48+(1000*2/math.pi)*(math.asin((i+1)/n)-math.asin(i/n))) for i in range(n)][::-1]
	static const uint8_t delta[] = {
		234, 99, 77, 66, 59, 55, 52, 49, 47, 46, 44, 44, 43, 43, 42};
	while(1) {
		for (int i = 0; i < 15; i++) {
			led_set_brightness(led, 0, (15-i)*100/15);
			k_usleep(delta[i]*400);
		}
		for (int i = 0; i < 15; i++) {
			led_set_brightness(led, 0, i*100/15);
			k_usleep(delta[14-i]*400);
		}
	}
}

int main(void)
{
	printk("Hello World %s! [%s]\n", CONFIG_BOARD, __TIMESTAMP__);

	const struct device *const led = DEVICE_DT_GET(LED_NODE);
	if ( ! device_is_ready(led)) {
		printk("LED device not ready\n");
		return 0;
	}
	const struct device *const btn = DEVICE_DT_GET(BTN_NODE);
	if ( ! device_is_ready(btn)) {
		printk("Buttons device not ready\n");
		return 0;
	}

	// prepare LED breath thread
	static K_THREAD_STACK_DEFINE(breath_thread_stack, 500);
	struct k_thread breath_thread;
	k_tid_t breath_tid = 0;

	while (1) {
		// launch LED breath thread upon KC
		if (konami_code.active && ! breath_tid)
			breath_tid = k_thread_create(
				&breath_thread, breath_thread_stack,
				K_THREAD_STACK_SIZEOF(breath_thread_stack),
				breath_thread_func, NULL, NULL, NULL,
				5, 0, K_NO_WAIT);

		snake_update(led, &snake_data);
		
		// increase speed every 5 points
		unsigned speed = snake_data.base + (snake_data.points / 5);
		k_msleep(1400 / speed);
	}

	return 0;
}
