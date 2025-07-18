/*
 * Copyright (c) 2016 Intel Corporation
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <stdio.h>
#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/devicetree.h>

/* 1000 msec = 1 sec */
#define SLEEP_TIME_MS   1000


int main(void)
{


	while (1) {
		printf("Hello World %s!\n", CONFIG_BOARD);
		k_msleep(SLEEP_TIME_MS);
	}
	return 0;
}
