/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "kc.h"
#include "snake.h"
#include "led.h"

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define BTN_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_keys)



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

	boot_animation(led);

	// prepare LED breath thread
	static K_THREAD_STACK_DEFINE(breath_thread_stack, 500);
	struct k_thread breath_thread;
	k_tid_t breath_tid = 0;

	while (1) {
		// launch LED breath thread upon KC
		if (konami_code.active && ! breath_tid)
			breath_tid = k_thread_create(
				&breath_thread,                                  // new_thread
				breath_thread_stack,                             // stack
				K_THREAD_STACK_SIZEOF(breath_thread_stack),      // stack_size
				&breath_thread_func,                             // entry
				NULL,                                            // p1
				NULL,                                            // p2
				NULL,                                            // p3
				5,                                               // prio
				0,                                               // options
				K_NO_WAIT);                                      // delay

		snake_update(led, &snake_data);
		
		// increase speed every 5 points
		unsigned speed = snake_data.base + (snake_data.points / 5);
		k_msleep(1400 / speed);
	}

	return 0;
}
