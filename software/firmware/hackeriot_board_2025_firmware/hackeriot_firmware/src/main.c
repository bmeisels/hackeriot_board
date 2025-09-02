/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "led.h"
#include "maze.h"
#include "simon.h"
#include "snake.h"

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define BTN_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_keys)

#define GAME_SNAKE	0
#define GAME_SIMON	1
#define GAME_MAZE	2

uint8_t do_menu(const struct device *led)
{
	static const char * const menu_options[] = {
		"Snake ",	// keep terminal space
		"Simon ",	// keep terminal space
		"Maze ",	// keep terminal space
	};

	uint8_t menu_pos = 0;

	// TODO: implement menu navigation

	return menu_pos;
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

	boot_animation(led);

	uint8_t choice = do_menu(led);

	switch(choice) {
		case GAME_SNAKE:
			play_snake(led);
			break;	// not reachable

		case GAME_SIMON:
			play_simon(led);
			break;	// not reachable

		case GAME_MAZE:
			play_maze(led);
			break;	// not reachable
	}

	return 0;
}
