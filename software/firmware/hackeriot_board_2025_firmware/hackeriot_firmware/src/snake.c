/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "buttons.h"
#include "screen.h"
#include "snake.h"
#include "persist.h"

static bool snake_inside(struct snake_data_t *sd, uint8_t pos)
{
	for (unsigned i = 0; i < sd->len; i++)
		if (pos == sd->pos[i]) return true;
	return false;
}

static bool do_update(struct snake_data_t *sd)
{
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
		screen_pixel_off(sd->pos[sd->len - 1]);
	
	// update body
	for (unsigned i = sd->len - 1; i > 0; i--) {
		sd->pos[i] = sd->pos[i - 1];
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
	if (snake_inside(sd, head)) {
		printk("Crash at pos=%d\n", head);
		return false;
	} else {
		sd->pos[0] = head;
		screen_pixel_on(head);
	}

	// check if target reached
	if (head == sd->target_pos) {
		printk("Target at pos=%d acquired\n", head);

		++sd->grow;
		++sd->points;

		// randomize new target outside snake
		uint8_t tpos;
		do
			tpos = sys_rand8_get() & 63;
		while(snake_inside(sd, tpos));
		sd->target_pos = tpos;
		printk("New target at pos=%d\n", tpos);
		screen_pixel_blink(tpos, true);
	}
	return true;
}

unsigned play_snake()
{
	printk("[%s] new game\n", __func__);

	// init data
	struct snake_data_t sd = {
		.points		= 0,
		.direction	= sys_rand8_get() & 3,
		.len 		= 1,
		.grow		= INITIAL_SNAKE_LEN - 1,
		.base     	= INITIAL_SNAKE_SPEED,
		.target_pos = sys_rand8_get() & 63,
	};
	// randomize distinct target and snake
	sd.pos[0] = sd.target_pos ^ (sys_rand32_get() % 63);

	// init screen
	screen_set(0);
	screen_pixel_on(sd.pos[0]);
	screen_pixel_blink(sd.target_pos, true);

	// game loop
	while (do_update(&sd)) {
		// increase speed every 5 points
		unsigned speed = sd.base + (sd.points / 5);

		char btn = buttons_get("UDLR", K_MSEC(1400 / speed));
		switch (btn) {
			case 'U':	sd.direction = 0; break;
			case 'L':	sd.direction = 1; break;
			case 'D':	sd.direction = 2; break;
			case 'R':	sd.direction = 3; break;
		}
		//if (btn) printk("[%s] btn=%c\n", __func__, btn);
	}
	
	// erase snake and target
	for (unsigned i = 1; i <= sd.len; i++) {
		screen_pixel_off(sd.pos[sd.len - i]);
		k_msleep(100);
	}
	screen_pixel_off(sd.target_pos);

	printk("[%s] game ended, score=%u\n", __func__, sd.points);
	return sd.points;
}
