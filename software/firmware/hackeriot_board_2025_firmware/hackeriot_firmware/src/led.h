/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef __LED_H__
#define __LED_H__

#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)

#endif // __LED_H__