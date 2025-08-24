/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LED_H__
#define __LED_H__



void led_swipe(const struct device *led, uint64_t cur, uint64_t new,
	char direction, uint32_t delay);
void boot_animation(const struct device *led);
void breath_thread_func(void*, void*, void*);

#endif // __LED_H__