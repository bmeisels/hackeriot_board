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
	static const char * const emenu_options[] = {
		"1.Snake",
		"2.Simon",
		"3.Maze",
	};
	static const char * const hmenu_options[] = { 
		"1.סנייק",
		"2.סיימון",
		"3.מבוך",
	};
	const char * const *menu_options = hebrew ? hmenu_options : emenu_options;
	char dir = hebrew ? 'R' : 'L';

	uint8_t menu_pos = 0;

	uint64_t old = 0, cur;
    char const *menu_item = menu_options[menu_pos];
	unsigned i = 0;
	char ch;
	while (1) {
		ch = menu_item[i++];
		if (ch == 0xd7) ch = menu_item[i++];
		if ( ! ch) { i = 0; ch = ' '; }
		cur = led_glyph(ch);
		led_swipe(led, old, cur, dir, 50);
		old = cur;

		ch = buttons_get("UDAB", K_NO_WAIT);
		unsigned menu_pos_step = ARRAY_SIZE(emenu_options);
		switch(ch) {
			case 'A':
				led_swipe(led, old, 0, dir, 50);
				return menu_pos;
			case 'B':
				hebrew = ! hebrew;
				ch = dir = hebrew ? 'R' : 'L';
				menu_options = hebrew ? hmenu_options : emenu_options;
				break;
			case 'D':
				++menu_pos_step;
				break;
			case 'U':
				--menu_pos_step;
				break;
			default:
			 	continue;
		}
		menu_pos = (menu_pos + menu_pos_step) % ARRAY_SIZE(emenu_options);
		menu_item = menu_options[menu_pos];
		i = 0;
		cur = led_glyph(menu_item[i++]);
		led_swipe(led, old, cur, ch, 50);
		old = cur;
	}

	// never reached
}

bool show_score(const struct device *led, uint8_t points)
{
	char emsg[10] = "Score:";
	char hmsg[20] = "ניקוד:";
	char *msg = hebrew ? hmsg : emsg;
	char dir = hebrew ? 'R' : 'L';

	// uint8_t to bidi string
	if (points < 10) {
		msg[hebrew ? 11 : 6] = '0' + (points % 10);
	} else {
		msg[hebrew ? 12 : 6] = '0' + (points / 10);
		msg[hebrew ? 11 : 7] = '0' + (points % 10);
	}
	unsigned i = 0;
	uint64_t old = 0;
	while(1) {
		char ch = msg[i++];
		if (ch == 0xd7) ch = msg[i++];
		if ( ! ch) { i = 0; ch = ' '; }
		uint64_t cur = led_glyph(ch);
		led_swipe(led, old, cur, dir, 50);
		old = cur;

		ch = buttons_get("AB", K_NO_WAIT);
		if (ch == 'A') return true;
		if (ch == 'B') return false;
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

		unsigned points = 0;
		do {
			// clear screen and blink
			led_blink(led, 0, 0, 0);
			led_set_brightness(led, 0, 100);
			for (unsigned i = 0; i < 64; i++) {
				led_off(led, POS_TO_LED(i));
			#ifdef BREADBOARD
				led_off(led, POS_TO_LED(i) | 8);
			#endif
			}

			switch(choice) {
				case GAME_SNAKE:
					points = play_snake(led);
					break;

				case GAME_SIMON:
					points = play_simon(led);
					break;

				case GAME_MAZE:
					points = play_maze(led);
					break;
			}
		} while(show_score(led, points));

	}

	return 0;
}
