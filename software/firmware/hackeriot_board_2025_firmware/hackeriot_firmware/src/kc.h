/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __KC_H__
#define __KC_H__



struct konami_code_t {
	unsigned int state	: 4;	// number of matches so far
	unsigned int active	: 1;
};

#endif // __KC_H__
