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
#include "maze.h"
#include "screen.h"
#include "simon.h"
#include "snake.h"

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define BTN_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_keys)

#define GAME_SNAKE	0
#define GAME_SIMON	1
#define GAME_MAZE	2

void boot_animation()
{
	printk("boot animation started\n");

	const char *msg[] = {"Hackeriot 2025", "האקריות 5202"};
	bool skip = screen_scroll_once(msg[lang], LANG_DIR, K_MSEC(50), NULL);
	if ( ! skip) k_msleep(200);

	printk("boot animation %sed\n", skip ? "skipp" : "finish");
}

uint8_t do_menu()
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
	uint8_t menu_pos = 0;
	const char *msg;
	char dir = 'D';
	while (1) {
		switch(lang) {
			case LANG_EN: msg = emenu_options[menu_pos]; break;
			case LANG_HE: msg = hmenu_options[menu_pos]; break;
			default: msg = "?";
		}
		// all this mess is just to get a different swipe direction for the first letter
		char        btn = screen_swipe(get_glyph(msg[0]), dir,  K_MSEC(50), "UDAB");
		if ( ! btn) btn = screen_scroll_once(msg + 1, LANG_DIR, K_MSEC(50), "UDAB");
		if ( ! btn) btn = screen_swipe(0,             LANG_DIR, K_MSEC(50), "UDAB");
		if ( ! btn) btn = screen_scroll_infinite(msg, LANG_DIR, K_MSEC(50), "UDAB");
		unsigned menu_pos_step = ARRAY_SIZE(emenu_options);
		switch(btn) {
			case 'A':
				screen_swipe(0, LANG_DIR, K_MSEC(50), "");
				return menu_pos;
			case 'B': lang = (lang + 1) % LANG_END;	break;
			case 'D': ++menu_pos_step; break;
			case 'U': --menu_pos_step; break;
		}
		dir = (btn == 'B') ? LANG_DIR : btn; // 'U' and 'D' remain
		menu_pos = (menu_pos + menu_pos_step) % ARRAY_SIZE(emenu_options);
	}

	// never reached
}

bool show_score(uint8_t points)
{
	char emsg[10] = "Score:";
	char hmsg[20] = "ניקוד:";
	// bidi-aware itoa
	if (points < 10) {
		emsg[6] = hmsg[11] = '0' + (points % 10);
	} else {
		emsg[6] = hmsg[12] = '0' + (points / 10);
		emsg[7] = hmsg[11] = '0' + (points % 10);
	}

	char *msg;
	switch(lang) {
		case LANG_EN: msg = emsg; break;
		case LANG_HE: msg = hmsg; break;
		default: msg = "?";
	}

	char btn = screen_scroll_infinite(msg, LANG_DIR, K_MSEC(50), "AB");
	return (btn == 'A');
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

	boot_animation();

	while (1) {
		uint8_t choice = do_menu();
		printk("Menu selection: %d\n", choice);

		unsigned points = 0;
		do {
			switch(choice) {
				case GAME_SNAKE:
					points = play_snake();
					break;

				case GAME_SIMON:
					points = play_simon();
					break;

				case GAME_MAZE:
					points = play_maze();
					break;
			}
		} while(show_score(points));

	}

	return 0;
}
