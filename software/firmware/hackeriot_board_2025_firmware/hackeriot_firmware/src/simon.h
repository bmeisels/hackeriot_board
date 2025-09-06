/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __SIMON_H__
#define __SIMON_H__

#define INITIAL_SIMON_LEN 3
#define SIMON_DELAY 700
#define SIMON_MAX_LEN 30

#define SIMON_GLYPH_OK 0x0065959696956500ULL

struct simon_data_t {
	uint8_t points;
    uint8_t len;
    char seq[SIMON_MAX_LEN];
};

void play_simon(const struct device *led);

#endif // __SIMON_H__
