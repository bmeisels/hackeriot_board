/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/led.h>
#include <zephyr/input/input.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "snake.h"

void snake_button_cb(struct input_event *evt, void *userdata)
{
	struct snake_data_t *const sd = userdata;
	if ( ! evt->value) return; // ignore button release
	switch (evt->code) {
		case INPUT_BTN_DPAD_UP:		sd->direction = 0; break;
		case INPUT_BTN_DPAD_LEFT:	sd->direction = 1; break;
		case INPUT_BTN_DPAD_DOWN:	sd->direction = 2; break;
		case INPUT_BTN_DPAD_RIGHT:	sd->direction = 3; break;
		case INPUT_BTN_A:			sd->pause = ! sd->pause; break;
		case INPUT_BTN_B:			++sd->base; break;
	}
	//printk("dir=%c pause=%c\n", 
	//	"ULDR"[sd->direction], sd->pause ? 'Y' : 'N');
}

struct snake_data_t snake_data;
INPUT_CALLBACK_DEFINE(NULL, snake_button_cb, &snake_data);

void snake_data_init(struct snake_data_t *sd)
{
	sd->points		= 0;
	sd->direction	= sys_rand8_get() & 3;
	sd->len			= 1;
	sd->grow		= INITIAL_SNAKE_LEN - 1;
    sd->base        = INITIAL_SNAKE_SPEED;
	sd->pause		= false;
	// randomize distinct target and snake
	sd->pos[0]		= sys_rand8_get() & 63;
	sd->target_pos	= sd->pos[0] ^ (sys_rand32_get() % 63);
}

inline bool snake_inside(struct snake_data_t *sd, uint8_t pos)
{
	for (unsigned i = 0; i < sd->len; i++)
		if (pos == sd->pos[i]) return true;
	return false;
}

// for Benny's dual-colored HT16K33
#define POS_TO_LED(x) ((x&7) | ((x&~7) << 1))

void snake_update(const struct device *led, struct snake_data_t *sd)
{
	if (sd->pause) return;	// no updates while paused

	if (! sd->len) {  // new game
		// clear screen and blink
		led_blink(led, 0, 0, 0);
		led_set_brightness(led, 0, 100);
		for (unsigned i = 0; i < 64; i++) {
			led_off(led, POS_TO_LED(i));
			led_off(led, POS_TO_LED(i) | 8); // TODO: monochrome fix
		}
		snake_data_init(sd);
		led_on(led, POS_TO_LED(sd->pos[0]));

		unsigned tpos = sd->target_pos;
		led_on(led, POS_TO_LED(tpos) | 8); // TODO: monochrome fix

		printk("New snake game\n");
	} 

	// debug prints
	printk("points=%u target=%u len=%u grow=%u dir=%c pos=", 
		sd->points, sd->target_pos, sd->len, sd->grow, "ULDR"[sd->direction]);
	for (unsigned i = 0; i < sd->len; i++)
		printk("%d ", sd->pos[i]);
	printk("\n");

	// update tail
	if (sd->grow) {
		++sd->len;
		--sd->grow;
	} else 
		led_off(led, POS_TO_LED(sd->pos[sd->len - 1]));
	
	// update body
	for (unsigned i = sd->len - 1; i > 0; i--) {
		sd->pos[i] = sd->pos[i-1];
	}
	
	// update head
	unsigned head = sd->pos[0];
	switch(sd->direction) {
		case 0:
			head = (head + 8) & 63;
			break;
		case 1:
			head += ((head & 7) == 7) ? -7 : 1;
			break;
		case 2:
			head = (head + 56) & 63;
			break;
		case 3:
			head += (head & 7) ? -1 : 7;
			break;
	}
	// handle crash
	if (snake_inside(sd, head)) {
		printk("Crash at pos=%d\n", head);
		sd->len = 0;
		sd->pause = true;
		led_blink(led, 0, 500, 500);
		led_set_brightness(led, 0, 100);
	} else {
		sd->pos[0] = head;
		led_on(led, POS_TO_LED(head));
	}
	// check if target reached
	if (head == sd->target_pos) {
		printk("Target at pos=%d acquired\n", head);
		led_off(led, POS_TO_LED(head)|8);

		++sd->grow;
		++sd->points;

		// randomize new target outside snake
		uint8_t tpos;
		do
			tpos = sys_rand8_get() & 63;
		while(snake_inside(sd, tpos));
		sd->target_pos = tpos;
		printk("New target at pos=%d\n", tpos);
		led_on(led, POS_TO_LED(tpos)|8);
	}
}
