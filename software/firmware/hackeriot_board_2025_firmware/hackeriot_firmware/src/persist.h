/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __PERSISTENCE_H__
#define __PERSISTENCE_H__

#include <zephyr/kernel.h>

enum language {
    LANG_EN = 0,
    LANG_HE = 1,
    LANG_END /* keep last */
};

extern struct eeprom_settings_t {
	uint32_t		magic;
	unsigned int	lang		: 3;	// actual type: enum language
	unsigned int	brightness	: 4;	// 1 to 15
	unsigned int	speed		: 7;	// scroll speed in ms, higher is slower
} settings;

struct highscore_t {
	unsigned int	play_count	: 20;
	unsigned int	score		: 9;
	unsigned int	lang		: 3;	// actual type: enum language
	char			name[28];
};

#define N_GAMES				3
#define EEPROM_HS_OFFSET    32
#define EEPROM_MAGIC        0x48485257UL /* 'HHRW' */
#define LANG_DIR			"LR"[settings.lang]
#define PIXEL_DELAY			K_MSEC(settings.speed)

void persist_load_settings(const struct device *eeprom);
void persist_save_settings(const struct device *eeprom);
void persist_save_highscore(const struct device *eeprom, uint8_t idx,
    const struct highscore_t *hs);
void persist_reset_all(const struct device *eeprom);

#endif // __PERSISTENCE_H__