/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>

#include "kc.h"

static void kc_button_cb(struct input_event *evt, void *userdata)
{
	struct konami_code_t * const kc = userdata;

	if ( ! evt->value) return; // ignore button release

	static const uint16_t konami_code[] = {
		INPUT_BTN_DPAD_UP,
		INPUT_BTN_DPAD_UP,
		INPUT_BTN_DPAD_DOWN,
		INPUT_BTN_DPAD_DOWN,
		INPUT_BTN_DPAD_LEFT,
		INPUT_BTN_DPAD_RIGHT,
		INPUT_BTN_DPAD_LEFT,
		INPUT_BTN_DPAD_RIGHT,
		INPUT_BTN_B,
		INPUT_BTN_A
	};

	unsigned st = kc->state;
	uint16_t code = evt->code;
	if (code == konami_code[st]) {
		++st;
		if (st == ARRAY_SIZE(konami_code)) {
			st = 0;
			kc->active = ! kc->active;
			printk("Konami code entered\n");
		}
	} else if (code == konami_code[0]) {
		if (st != 2) st = 1;	// 2 stays 2
	} else st = 0;
	kc->state = st;

	if (st > 2)
		printk("state=%u active=%c\n", st, "NY"[kc->active]);
}

struct konami_code_t konami_code;

INPUT_CALLBACK_DEFINE(NULL, kc_button_cb, &konami_code);
