/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SCREEN_H__
#define __SCREEN_H__

#include <zephyr/kernel.h>

// the following are in Hertz
#define SCREEN_FPS          50
#define SCREEN_BLINK_FAST   10
#define SCREEN_BLINK_SLOW   2

// blinkall functions
enum blink_speed {
    BLINK_NONE = 0,
    BLINK_2HZ = 250,
    BLINK_1HZ = 500,
    BLINK_05HZ = 1000
};
void screen_blinkall(enum blink_speed bs);

// mask functions
void screen_mask_on(uint64_t mask);
void screen_mask_off(uint64_t mask);
void screen_mask_invert(uint64_t mask);
void screen_mask_blink(uint64_t mask, bool fast);

// pixel functions
inline void screen_pixel_on(uint8_t pos)
{
    screen_mask_on(1ULL << pos);
}
inline void screen_pixel_off(uint8_t pos)
{
    screen_mask_off(1ULL << pos);
}
inline void screen_pixel_invert(uint8_t pos)
{
    screen_mask_invert(1ULL << pos);
}
inline void screen_pixel_blink(uint8_t pos, bool fast)
{
    screen_mask_blink(1ULL << pos, fast);
}

// bitmap functions
uint64_t get_glyph(char ch);

void screen_set(uint64_t bitmap);

char screen_swipe(uint64_t bitmap, char direction, 
    k_timeout_t pixel_delay, const char *buttons);

// text functions
char screen_scroll_once(const char *text, char direction, 
    k_timeout_t pixel_delay, const char *buttons);

char screen_scroll_infinite(const char *text, char direction, 
    k_timeout_t pixel_delay, const char *buttons);

#endif // __SCREEN_H__