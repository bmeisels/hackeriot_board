/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/input/input.h>
#include <zephyr/sys/printk.h>

#include "buttons.h"

K_PIPE_DEFINE(buttons_pipe, 16, 1); // 16 bytes, not aligned

static const char all_allowed[] = "UDLRABudlrab";

char buttons_get(const char *filter, k_timeout_t timeout)
{
    k_timepoint_t tp = sys_timepoint_calc(timeout);
    uint8_t ch;
    if ( ! filter) filter = all_allowed;
    while (1) {
        int res = k_pipe_read(&buttons_pipe, &ch, 1, sys_timepoint_timeout(tp));
        if (res == -EAGAIN) // timeout expired
            return 0;
        if (res == -EPIPE || res == -ECANCELED) {
            printk("Cannot read from pipe; it is %s\n", 
                res == -EPIPE ? "closed" : "reset");
            return 0;
        }
        if (res == 1 && strchr(filter, ch))
            return ch;
    }
    // never reached
}

void buttons_pipe_cb(struct input_event *evt, void *)
{
    char ch;
	switch (evt->code) {
		case INPUT_BTN_DPAD_UP:		ch = 'U'; break;
		case INPUT_BTN_DPAD_LEFT:	ch = 'L'; break;
		case INPUT_BTN_DPAD_DOWN:	ch = 'D'; break;
		case INPUT_BTN_DPAD_RIGHT:	ch = 'R'; break;
		case INPUT_BTN_A:			ch = 'A'; break;
		case INPUT_BTN_B:			ch = 'B'; break;
        default:                    ch = 'Z';
	}
	if ( ! evt->value)
        ch ^= BIT(6); // lowercase = button release

    int res = k_pipe_write(&buttons_pipe, &ch, 1, K_NO_WAIT);
    if (res != 1) {
        printk("Cannot write '%c' to pipe; it is ", ch);
        if (res == -ECANCELED)      printk("reset\n");
        else if (res == -EAGAIN)    printk("full\n");
        else if (res == -EPIPE)     printk("closed\n");
        else                        printk("error %d\n", res);
    }
}

INPUT_CALLBACK_DEFINE(NULL, buttons_pipe_cb, NULL);
