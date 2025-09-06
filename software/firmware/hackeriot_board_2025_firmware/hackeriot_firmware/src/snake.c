/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "buttons.h"
#include "led.h"
#include "snake.h"

inline bool snake_inside(struct snake_data_t *sd, uint8_t pos)
{
	for (unsigned i = 0; i < sd->len; i++)
		if (pos == sd->pos[i]) return true;
	return false;
}

static bool do_update(const struct device *led, struct snake_data_t *sd)
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
	if (snake_inside(sd, head)) {
		printk("Crash at pos=%d\n", head);
		return false;
	}

	sd->pos[0] = head;
	led_on(led, POS_TO_LED(head));

	// check if target reached
	if (head == sd->target_pos) {
		printk("Target at pos=%d acquired\n", head);
#ifdef BREADBOARD
		led_off(led, POS_TO_LED(head) | 8);
#endif

		++sd->grow;
		++sd->points;

		// randomize new target outside snake
		uint8_t tpos;
		do
			tpos = sys_rand8_get() & 63;
		while(snake_inside(sd, tpos));
		sd->target_pos = tpos;
		printk("New target at pos=%d\n", tpos);
#ifdef BREADBOARD
		led_on(led, POS_TO_LED(tpos) | 8);
#else
		led_on(led, POS_TO_LED(tpos));
#endif
	}
	return true;
}

unsigned play_snake(const struct device *led)
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

	// init board
	led_on(led, POS_TO_LED(sd.pos[0]));

	#ifdef BREADBOARD
	led_on(led, POS_TO_LED(sd.target_pos) | 8);
	#else
	led_on(led, POS_TO_LED(sd.target_pos));
	#endif

	while (do_update(led, &sd)) {
		// increase speed every 5 points
		unsigned speed = sd.base + (sd.points / 5);

		char ch = buttons_get("UDLR", K_MSEC(1400 / speed));
		switch (ch) {
			case 'U':	sd.direction = 0; break;
			case 'L':	sd.direction = 1; break;
			case 'D':	sd.direction = 2; break;
			case 'R':	sd.direction = 3; break;
		}
		//if (ch) printk("[%s] ch=%c\n", __func__, ch);
	}
	
	// erase snake and target
	for (unsigned i = 1; i <= sd.len; i++) {
		led_off(led, POS_TO_LED(sd.pos[sd.len - i]));
		k_msleep(100);
	}
	#ifdef BREADBOARD
	led_off(led, POS_TO_LED(sd.target_pos) | 8);
	#else
	led_off(led, POS_TO_LED(sd.target_pos));
	#endif

	printk("[%s] game ended, score=%u\n", __func__, sd.points);
	return sd.points;
}
