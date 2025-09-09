/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/drivers/eeprom.h>
#include <zephyr/sys/printk.h>

#include "persist.h"

#define		DEFAULT_BRIGHTNESS	10
#define		DEFAULT_LANG		LANG_HE
#define		DEFAULT_SPEED		70

struct eeprom_settings_t settings = {
	.magic = EEPROM_MAGIC,
	.brightness = DEFAULT_BRIGHTNESS,
	.lang = DEFAULT_LANG,
	.speed = DEFAULT_SPEED,
};

void persist_load_settings(const struct device *eeprom)
{
	printk("EEPROM size=%zu.\n", eeprom_get_size(eeprom));

	int rc = eeprom_read(eeprom, 0, &settings, sizeof(settings));
	if (rc < 0) {
		printk("[%s] header read error; code=%d.\n", __func__, rc);
	}
	if (settings.magic == EEPROM_MAGIC) {
		// validate language
		if (settings.lang >= LANG_END) {
    		printk("[%s] invalid lang %d.\n", __func__, settings.lang);
			settings.lang = LANG_HE;
		}
		// validate speed
		if (settings.speed < 40) {
    		printk("[%s] invalid speed %d.\n", __func__, settings.speed);
			settings.speed = 40;
		}

	} else {
		printk("[%s] magic not found; resetting EEPROM.\n", __func__);
		persist_reset_all(eeprom);
	}
}

void persist_save_settings(const struct device *eeprom)
{
	int rc = eeprom_write(eeprom, 0, &settings, sizeof(settings));
	if (rc < 0) {
		printk("[%s] write error; code=%d.\n", __func__, rc);
	}
}

void persist_save_highscore(const struct device *eeprom, uint8_t idx,
    const struct highscore_t *hs)
{
    int rc = eeprom_write(eeprom, EEPROM_HS_OFFSET + idx * sizeof(*hs),
        hs, sizeof(*hs));
    if (rc < 0) {
        printk("[%s] write error; idx=%u code=%d.\n", __func__, idx, rc);
    }
}

void persist_reset_all(const struct device *eeprom)
{
	struct highscore_t hs = {
		.play_count	= 0,
		.score 		= 1,
		.lang		= LANG_HE,
		.name		= "מהממת",
	};

	// reset highscores
	for (unsigned i = 0; i < N_GAMES; i++)
        persist_save_highscore(eeprom, i, &hs);

	// reset settings
	settings.magic = EEPROM_MAGIC;
	settings.brightness = DEFAULT_BRIGHTNESS;
	settings.lang = DEFAULT_LANG;
	settings.speed = DEFAULT_SPEED;
	persist_save_settings(eeprom);
}

