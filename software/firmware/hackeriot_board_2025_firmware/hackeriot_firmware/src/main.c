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
#include "persist.h"
//#include "pong.h"
#include "screen.h"
#include "simon.h"
#include "snake.h"

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define BTN_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(gpio_keys)
#define EEP_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(atmel_at24)

enum main_menu {
	MENU_SNAKE,
	MENU_SIMON,
	MENU_PONG,
	MENU_SETTINGS,
	MENU_END		// keep last
};

void boot_animation()
{
	printk("boot animation started\n");

	const char *msg[] = {"Hackeriot 2025", "האקריות 5202"};
	bool skip = screen_scroll_once(msg[settings.lang], LANG_DIR, PIXEL_DELAY, NULL);
	if ( ! skip) k_msleep(200);

	printk("boot animation %sed\n", skip ? "skipp" : "finish");
}

uint8_t do_menu()
{
	static const char * const emenu_options[] = {
		"1.Snake",
		"2.Simon",
		"3.Pong",
		"4.Settings",
	};
	static const char * const hmenu_options[] = { 
		"1.סנייק",
		"2.סיימון",
		"3.פונג",
		"4.אפשרויות",
	};
	uint8_t menu_pos = 0;
	const char *msg;
	char dir = 'D';
	while (1) {
		switch(settings.lang) {
			case LANG_EN: msg = emenu_options[menu_pos]; break;
			case LANG_HE: msg = hmenu_options[menu_pos]; break;
			default: msg = "?";
		}
		// all this mess is just to get a different swipe direction for the first letter
		char        btn = screen_swipe(get_glyph(msg[0]), dir,  PIXEL_DELAY, "UDAB");
		if ( ! btn) btn = screen_scroll_once(msg + 1, LANG_DIR, PIXEL_DELAY, "UDAB");
		if ( ! btn) btn = screen_swipe(0,             LANG_DIR, PIXEL_DELAY, "UDAB");
		if ( ! btn) btn = screen_scroll_infinite(msg, LANG_DIR, PIXEL_DELAY, "UDAB");
		unsigned menu_pos_step = ARRAY_SIZE(emenu_options);
		switch(btn) {
			case 'A':
				screen_swipe(0, LANG_DIR, PIXEL_DELAY, "");
				return menu_pos;
			case 'B': settings.lang = (settings.lang + 1) % LANG_END;	break;
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
	switch(settings.lang) {
		case LANG_EN: msg = emsg; break;
		case LANG_HE: msg = hmsg; break;
		default: msg = "?";
	}

	char btn = screen_scroll_infinite(msg, LANG_DIR, PIXEL_DELAY, "AB");
	return (btn == 'A');
}

uint64_t thin_number_glyph(uint8_t n)
{
	static const uint64_t digits_glyph[] = {
		0x708898a8c8887000,	// 0
		0x2060202020207000,	// 1
		0x708808102040f800,	// 2
		0x7088081008887000, // 3
		0x10305090f8101000, // 4
		0xf88080f008887000,	// 5
		0x204080f088887000, // 6
		0xf808102040404000, // 7
		0x7088887088887000, // 8
		0x7088887808087000, // 9
	};
	uint64_t bitmap = digits_glyph[n%10];
	if (n < 10)
		return bitmap >> 2;
	else
		return (bitmap >> 3) | (digits_glyph[1] << 1);
}

bool do_settings_language()
{
	static const char * const elang_options[] = {"English", "Hebrew"};
	static const char * const hlang_options[] = {"אנגלית", "עברית"};

	const char *msg;
	uint8_t lang = settings.lang;
	while (1) {
		switch(settings.lang) {
			case LANG_EN: msg = elang_options[lang]; break;
			case LANG_HE: msg = hlang_options[lang]; break;
			default: msg = "?";
		}
		char btn = screen_scroll_infinite(msg, LANG_DIR, PIXEL_DELAY, "LRAB");
		screen_swipe(0, LANG_DIR, PIXEL_DELAY, "");
		switch(btn) {
			case 'L': 
				if (lang) --lang;
				break;

			case 'R':
			 	if (lang < LANG_END-1) ++lang;
				break;

			case 'A':
				if (settings.lang != lang) {
					settings.lang = lang;
					return true;
				}
				// fall-through
				
			case 'B':
			 	return false;
		}
		printk("lang=%u\n", lang);
	}
}

bool do_settings_brightness(const struct device *led)
{
	unsigned int brightness = settings.brightness;

	char dir = 'L';
	while (1) {
		const char *filter = brightness ? (brightness >= 15 ? "LAB" : "LRAB") : "RAB";
		char btn = screen_swipe(thin_number_glyph(brightness), dir, PIXEL_DELAY, filter);
		if ( ! btn) btn = buttons_get(filter, K_FOREVER);
		switch(btn) {
			case 'L':	--brightness; break;
			case 'R':	++brightness; break;
			case 'A':
				if (settings.brightness != brightness) {
					settings.brightness = brightness;
					return true;
				}
				// fall-through
			case 'B':
				led_set_brightness(led, 0, settings.brightness*100/15);
				return false;
		}
		led_set_brightness(led, 0, brightness*100/15);

		dir = btn;
		printk("brightness=%u\n", brightness);
	}
}

bool do_settings_speed()
{
	unsigned int speed = settings.speed;

	char dir = 'L';
	while (1) {
		const char *filter = (speed <= 35) ? "LAB" : ((speed >= 110) ? "RAB" : "LRAB");
		char btn = screen_swipe(thin_number_glyph(22-(speed / 5)), dir, PIXEL_DELAY, filter);
		if ( ! btn) btn = buttons_get(filter, K_FOREVER);
		switch(btn) {
			case 'R': speed -= 5; break;
			case 'L': speed += 5; break;
			case 'A':
				if (settings.speed != speed) {
					settings.speed = speed;
					return true;
				}
				// fall-through			
			case 'B':
				return false;
		}
		dir = btn;
		printk("speed=%u\n", speed);
	}
}


void do_settings_menu(const struct device *eeprom, const struct device *led)
{
	static const char * const emenu_options[] = {
		"1.Language",
		"2.Screen brightness",
		"3.Scroll speed",
		"4.Reset to default",
	};
	static const char * const hmenu_options[] = { 
		"1.שפה",
		"2.בהירות מסך",
		"3.מהירות תצוגה",
		"4.חזרה לברירת מחדל",
	};
	uint8_t menu_pos = 0;
	const char *msg;
	char dir = 'D';
	while (1) {
		switch(settings.lang) {
			case LANG_EN: msg = emenu_options[menu_pos]; break;
			case LANG_HE: msg = hmenu_options[menu_pos]; break;
			default: msg = "?";
		}
		// all this mess is just to get a different swipe direction for the first letter
		char        btn = screen_swipe(get_glyph(msg[0]), dir,  PIXEL_DELAY, "UDAB");
		if ( ! btn) btn = screen_scroll_once(msg + 1, LANG_DIR, PIXEL_DELAY, "UDAB");
		if ( ! btn) btn = screen_swipe(0,             LANG_DIR, PIXEL_DELAY, "UDAB");
		if ( ! btn) btn = screen_scroll_infinite(msg, LANG_DIR, PIXEL_DELAY, "UDAB");
		screen_swipe(0, LANG_DIR, PIXEL_DELAY, "");
		unsigned menu_pos_step = ARRAY_SIZE(emenu_options);
		switch(btn) {
			case 'D': ++menu_pos_step; break;
			case 'U': --menu_pos_step; break;
			case 'A':
				bool save = false;
				switch(menu_pos) {
					case 0: save = do_settings_language(); break;
					case 1: save = do_settings_brightness(led); break;
					case 2: save = do_settings_speed(); break;
					case 3: persist_reset_all(eeprom); break;
				}
				if (save) {
					persist_save_settings(eeprom);
					printk("Settings saved.\n");
				}
				break;
				
			case 'B': 
				return;
		}
		dir = (btn == 'A') ? LANG_DIR : btn; // 'U' and 'D' remain
		menu_pos = (menu_pos + menu_pos_step) % ARRAY_SIZE(emenu_options);
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
	const struct device *const eeprom = DEVICE_DT_GET(EEP_NODE);
	if ( ! device_is_ready(eeprom)) {
		printk("EEPROM device not ready\n");
		return 0;
	}

	persist_load_settings(eeprom);

	led_set_brightness(led, 0, settings.brightness*100/15);

	boot_animation();

	while (1) {
		uint8_t choice = do_menu();
		printk("Menu selection: %d\n", choice);

		switch(choice) {
			case MENU_SNAKE:
				while(show_score(play_snake())) {}
				break;

			case MENU_SIMON:
				while(show_score(play_simon())) {}
				break;

			case MENU_PONG:
				//while(show_score(play_pong())) {}
				//break;
				const char *msg[] = {"Not implemented", "טרם מומש"};
				screen_scroll_once(msg[settings.lang], LANG_DIR, PIXEL_DELAY, "AB");
				break;

			case MENU_SETTINGS:
				do_settings_menu(eeprom, led);
				break;
		}
	}

	return 0;
}
