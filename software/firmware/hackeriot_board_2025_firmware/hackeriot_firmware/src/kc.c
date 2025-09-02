/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>

#include "kc.h"
#include "led.h"

struct konami_code_t konami_code;

// LED breath thread globals
static K_THREAD_STACK_DEFINE(breath_thread_stack, 500);
static struct k_thread breath_thread;
static k_tid_t breath_tid = 0;

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
			printk("Konami code entered\n");
			st = 0;
			kc->active = ! kc->active;

			if (kc->active) {
				// start/resume LED breath thread
				if ( ! breath_tid)
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
				else
					k_thread_resume(breath_tid);
			} else {
				// suspend thread
				k_thread_suspend(breath_tid);
			}
		}
	} else if (code == konami_code[0]) {
		if (st != 2) st = 1;	// 2 stays 2
	} else st = 0;
	kc->state = st;

	if (st > 2)
		printk("state=%u active=%c\n", st, "NY"[kc->active]);
}

INPUT_CALLBACK_DEFINE(NULL, kc_button_cb, &konami_code);
