/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SNAKE_H__
#define __SNAKE_H__


#define MAX_SNAKE_LEN 60
#define INITIAL_SNAKE_LEN 3
#define INITIAL_SNAKE_SPEED 2

struct snake_data_t {
	unsigned points 	: 6; 
	unsigned direction 	: 2; 	// 0=up, 1=left, 2=down, 3=right
	unsigned len 		: 6;
	unsigned grow 		: 2; 	// assuming INITIAL_SNAKE_LEN < 4
	unsigned target_pos	: 6;
	unsigned base		: 3; 	// base speed
	unsigned pause 		: 1;
	uint8_t pos[MAX_SNAKE_LEN]; // head first
};

void play_snake(const struct device *led);

#endif // __SNAKE_H__
