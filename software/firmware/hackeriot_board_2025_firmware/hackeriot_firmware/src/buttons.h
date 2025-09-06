/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __BUTTONS_H__
#define __BUTTONS_H__

#include <zephyr/kernel.h>

char buttons_get(const char *filter, k_timeout_t timeout);
void buttons_clear();

#endif // __BUTTONS_H__
