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
#include "screen.h"
#include "simon.h"
#include "persist.h"

#define SIMON_OPTIONS   "UDLRAB"

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
        case 0:     return 0x7EC391919D81C37E; // clock
        default:    return 0;
    };
}

static bool do_one_round(struct simon_data_t *sd)
{
    // display sequence
    printk("Simon:");
    for (unsigned i = 0; i < sd->len; i++) {
        char ch = sd->seq[i];
        printk(" %c", ch);
        screen_swipe(simon_glyph(ch), simon_dir(ch), PIXEL_DELAY, "");
        k_msleep(SIMON_DELAY);
    }
    printk("\n");
    buttons_clear();
    screen_swipe(get_glyph('?'), LANG_DIR, PIXEL_DELAY, "");

    bool game_on = true;
    // query sequence
    printk("Player:");
    for (unsigned i = 0; i < sd->len && game_on; i++) {
        char ch = buttons_get(SIMON_OPTIONS, K_MSEC(2 * SIMON_DELAY));
        if (ch) printk(" %c", ch); else printk(" (none)");
        uint64_t bitmap = simon_glyph(ch);
        screen_swipe(bitmap, simon_dir(ch), PIXEL_DELAY, "");
        if (ch != sd->seq[i]) game_on = false;
    }
    printk(" %s\n", game_on ? "OK" : "error");

    if (game_on) {
        screen_swipe(SIMON_GLYPH_OK, 'R', PIXEL_DELAY, "");
        k_msleep(2000);
    } else {
        screen_blinkall(BLINK_2HZ);
        k_msleep(3000);
        screen_blinkall(BLINK_NONE);
    }
    return game_on;
}

unsigned play_simon()
{
    printk("[%s] new game\n", __func__);
    struct simon_data_t sd = {
        .points = 0,
        .len = INITIAL_SIMON_LEN,
    };
    // randomize sequence
    for (unsigned i = 0; i < ARRAY_SIZE(sd.seq); i++)
        sd.seq[i] = SIMON_OPTIONS[sys_rand32_get() % (ARRAY_SIZE(SIMON_OPTIONS)-1)];

    // display Ready-3-2-1
    const char *msg[] = {"Ready?", "מוכנה?"};
    screen_scroll_once(msg[settings.lang], LANG_DIR, PIXEL_DELAY, "");
    for (const char *c = "321 "; *c; c++) {
        k_msleep(SIMON_DELAY);
        screen_swipe(get_glyph(*c), 'D', PIXEL_DELAY, "");
    }

    // game loop
    while(do_one_round(&sd))
    {
        ++sd.len;
        ++sd.points;
    }

    printk("[%s] game ended, score=%u\n", __func__, sd.points);
    return sd.points;
}
