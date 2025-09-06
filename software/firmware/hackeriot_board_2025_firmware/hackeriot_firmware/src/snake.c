/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/led.h>
#include <zephyr/random/random.h>
#include <zephyr/sys/printk.h>

#include "buttons.h"
#include "led.h"
#include "snake.h"

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

static void snake_update(const struct device *led, struct snake_data_t *sd)
{
	if (sd->pause) return;	// no updates while paused

	if (! sd->len) {  // new game
		// clear screen and blink
		led_blink(led, 0, 0, 0);
		led_set_brightness(led, 0, 100);
		for (unsigned i = 0; i < 64; i++) {
			led_off(led, POS_TO_LED(i));
#ifdef BREADBOARD
			led_off(led, POS_TO_LED(i) | 8);
#endif
		}
		snake_data_init(sd);
		led_on(led, POS_TO_LED(sd->pos[0]));

		unsigned tpos = sd->target_pos;
#ifdef BREADBOARD
		led_on(led, POS_TO_LED(tpos) | 8);
#else
		led_on(led, POS_TO_LED(tpos));
#endif

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
}

void play_snake(const struct device *led)
{
	struct snake_data_t sd = {.len = 0, .pause = 0};
	
	while (1) {
		snake_update(led, &sd);

		// increase speed every 5 points
		unsigned speed = sd.base + (sd.points / 5);

		char ch = buttons_get("UDLRAB", K_MSEC(1400 / speed));
		switch (ch) {
			case 'U':	sd.direction = 0; break;
			case 'L':	sd.direction = 1; break;
			case 'D':	sd.direction = 2; break;
			case 'R':	sd.direction = 3; break;
			case 'A':	sd.pause ^= 1; break;
			case 'B':	++sd.base; break;
		}
		//if (ch) printk("[%s] ch=%c\n", __func__, ch);

	}
}
