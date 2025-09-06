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
#include "simon.h"

static char simon_dir(char dir)
{
    switch (dir) {
        case 'U':
        case 'D':
        case 'L':
        case 'R':
            break;
        default:
            dir = "UDLR"[sys_rand8_get() & 3];            
    }
    //printf("[%s] dir=%c\n", __func__, dir);
    return dir;
}

static uint64_t simon_glyph(char ch)
{
    switch(ch) {
        case 'U':   return 0x183C7EFF18181818;
        case 'D':   return 0x18181818FF7E3C18;
        case 'L':   return 0x103070FFFF703010;
        case 'R':   return 0x080C0EFFFF0E0C08;
        case 'A':   return 0x3C7EE7C3FFFFC3C3;
        case 'B':   return 0xFEC3C3FEFEC3C3FE;
        default:    return 0;
    };
}

static bool do_one_round(const struct device *led, struct simon_data_t *sd)
{
    static const char options[] = "UDLRAB";

    // randomize sequence
    for (unsigned i = 0; i < sd->len; i++)
        sd->seq[i] = options[sys_rand32_get() % (ARRAY_SIZE(options)-1)];

    // display sequence
    uint64_t cur = 0, old;
    printk("Simon:");
    for (unsigned i = 0; i < sd->len; i++) {
        char ch = sd->seq[i];
        printk(" %c", ch);
        old = cur;
        cur = simon_glyph(ch);
        led_swipe(led, old, cur, simon_dir(ch), 50);
        k_msleep(SIMON_DELAY);
    }
    printk("\n");

    buttons_clear();

    old = cur;
    cur = led_glyph('?');
    led_swipe(led, old, cur, 'L', 50);

    bool game_on = true;
    // query sequence
    printk("Player:");
    for (unsigned i = 0; i < sd->len && game_on; i++) {
        char ch = buttons_get(options, K_MSEC(2 * SIMON_DELAY));
        if (ch) printk(" %c", ch); else printk(" (none)");
        old = cur;
        cur = simon_glyph(ch);
        if (ch != sd->seq[i]) {
            game_on = false;
            cur = ~cur;
            printk(" <-- error");
        }
        led_swipe(led, old, cur, simon_dir(ch), 50);
    }
    printk("\n");

    if (game_on) {
        k_msleep(SIMON_DELAY);
        led_swipe(led, cur, SIMON_GLYPH_OK, 'L', 50);
        printk("OK\n");
    }

    k_msleep(SIMON_DELAY * 3);

    return game_on;
}

unsigned play_simon(const struct device *led)
{
    printk("[%s] new game\n", __func__);
    struct simon_data_t sd = {.points = 0, .len=INITIAL_SIMON_LEN};

    uint64_t cur = 0, old;

    // display Ready321
	for (const char *c = "Ready?"; *c; ++c) {
		old = cur;
		cur = led_glyph(*c);
		led_swipe(led, old, cur, 'L', 50);
	}
    for (unsigned i = 3; i; --i) {
        k_msleep(SIMON_DELAY);
		old = cur;
		cur = led_glyph('0' + i);
		led_swipe(led, old, cur, 'D', 50);
    }
	led_swipe(led, cur, 0, 'D', 50);
    cur = 0;

    while(do_one_round(led, &sd))
    {
        ++sd.len;
        ++sd.points;

    	led_swipe(led, SIMON_GLYPH_OK, 0, 'L', 50);
    }

    printk("[%s] game ended, score=%u\n", __func__, sd.points);
    return sd.points;
}
