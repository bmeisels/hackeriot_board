/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/sys/printk.h>

#include "buttons.h"
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
		"1.Snake",
		"2.Simon",
		"3.Maze",
	};

	uint8_t menu_pos = 0;

	uint64_t old = 0, cur;
    char const *menu_item = menu_options[menu_pos];
	unsigned i = 0;
	char ch;
	while (1) {
		ch = menu_item[i++];
		if ( ! ch) { i = 0; ch = ' '; }
		cur = led_glyph(ch);
		led_swipe(led, old, cur, 'L', 50);
		old = cur;

		ch = buttons_get("UDA", K_NO_WAIT);
		unsigned menu_pos_step = ARRAY_SIZE(menu_options);
		switch(ch) {
			case 'A':
				led_swipe(led, old, 0, 'L', 50);
				return menu_pos;
			case 'D':
				++menu_pos_step;
				break;
			case 'U':
				--menu_pos_step;
				break;
			default:
			 	continue;
		}
		menu_pos = (menu_pos + menu_pos_step) % ARRAY_SIZE(menu_options);
		menu_item = menu_options[menu_pos];
		i = 0;
		cur = led_glyph(menu_item[i++]);
		led_swipe(led, old, cur, ch, 50);
		old = cur;
	}

	// never reached
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

	while (1) {
		uint8_t choice = do_menu(led);
		printk("Menu selection: %d\n", choice);

		switch(choice) {
			case GAME_SNAKE:
				play_snake(led);
				break;

			case GAME_SIMON:
				play_simon(led);
				break;

			case GAME_MAZE:
				play_maze(led);
				break;
		}
	}

	return 0;
}
