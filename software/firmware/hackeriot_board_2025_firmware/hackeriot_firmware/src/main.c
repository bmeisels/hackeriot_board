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

const uint8_t breath[] = {
	100, 100, 100, 100, 100, 99, 99, 99, 98, 98, 97, 96, 96, 95,
	94, 93, 92, 92, 90, 89, 88, 87, 86, 85, 83, 82, 81, 79, 78, 
	76, 74, 73, 71, 69, 67, 66, 64, 62, 60, 58, 56, 54, 52, 50, 
	48, 46, 43, 41, 39, 37, 34, 32, 30, 27, 25, 23, 20, 18, 16, 
	13, 11, 8, 6, 3, 1, 3, 6, 8, 11, 13, 16, 18, 20, 23, 25, 27, 
	30, 32, 34, 37, 39, 41, 43, 46, 48, 50, 52, 54, 56, 58, 60, 
	62, 64, 66, 67, 69, 71, 73, 74, 76, 78, 79, 81, 82, 83, 85, 
	86, 87, 88, 89, 90, 92, 92, 93, 94, 95, 96, 96, 97, 98, 98, 
	99, 99, 99, 100, 100, 100, 100
};

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

	unsigned tick = 0;
	while (1) {
		++tick;
		if ( ! (tick & 31)) snake_update(led, &snake_data);
		if (konami_code.active && snake_data.len)
			led_set_brightness(led, 0, breath[tick & 127]);

		// increase speed every 5 points
		unsigned speed = snake_data.base + (snake_data.points / 5);
		k_sleep(K_MSEC(48 / speed));
	}

	return 0;
}
