/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LED_H__
#define __LED_H__


#ifdef BREADBOARD
	// Adafruit's dual-colored HT16K33
	#define POS_TO_LED(x) ((x&7) | ((x&~7) << 1))
#else
	// board2025 monochromatic HT16K33, rotated
	#define POS_TO_LED(x) (((63-x)&7) | (((63-x)&~7) << 1))
#endif

void led_swipe(const struct device *led, uint64_t cur, uint64_t new,
	char direction, uint32_t delay);
uint64_t led_glyph(char c);
void boot_animation(const struct device *led);
void breath_thread_func(void*, void*, void*);

#endif // __LED_H__