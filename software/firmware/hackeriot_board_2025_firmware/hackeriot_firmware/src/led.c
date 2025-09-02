/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */

#include <zephyr/kernel.h>
#include <zephyr/devicetree.h>
#include <zephyr/drivers/led.h>
#include <zephyr/sys/printk.h>

#include "led.h"

#define LED_NODE DT_COMPAT_GET_ANY_STATUS_OKAY(holtek_ht16k33)
#define H_MASK 0x0101010101010101ULL

const uint64_t font_digits[] = {
	0x386CC6D6C66C3800UL,	// Char 048 (0)
	0x1838181818187E00UL,	// Char 049 (1)
	0x3C66061C30667E00UL,	// Char 050 (2)
	0x3C66061C06663C00UL,	// Char 051 (3)
	0x1C3C6CCCFE0C1E00UL,	// Char 052 (4)
	0x7E607C0606663C00UL,	// Char 053 (5)
	0x1C30607C66663C00UL,	// Char 054 (6)
	0x7E66060C18181800UL,	// Char 055 (7)
	0x3C66663C66663C00UL,	// Char 056 (8)
	0x3C66663E060C3800UL,	// Char 057 (9)
};

const uint64_t font_uppercase[] = {
	0x183C66667E666600UL,	// Char 065 (A)
	0xFC66667C6666FC00UL,	// Char 066 (B)
	0x3C66C0C0C0663C00UL,	// Char 067 (C)
	0xF86C6666666CF800UL,	// Char 068 (D)
	0xFE6268786862FE00UL,	// Char 069 (E)
	0xFE6268786860F000UL,	// Char 070 (F)
	0x3C66C0C0CE663E00UL,	// Char 071 (G)
	0x6666667E66666600UL,	// Char 072 (H)
	0x3C18181818183C00UL,	// Char 073 (I)
	0x1E0C0C0CCCCC7800UL,	// Char 074 (J)
	0xE6666C786C66E600UL,	// Char 075 (K)
	0xF06060606266FE00UL,	// Char 076 (L)
	0xC6EEFEFED6C6C600UL,	// Char 077 (M)
	0xC6E6F6DECEC6C600UL,	// Char 078 (N)
	0x7CC6C6C6C6C67C00UL,	// Char 079 (O)
	0xFC66667C6060F000UL,	// Char 080 (P)
	0x7CC6C6C6C6CE7C0EUL,	// Char 081 (Q)
	0xFC66667C6C66E600UL,	// Char 082 (R)
	0x3C6670380E663C00UL,	// Char 083 (S)
	0x7E5A181818183C00UL,	// Char 084 (T)
	0x6666666666667E00UL,	// Char 085 (U)
	0x66666666663C1800UL,	// Char 086 (V)
	0xC6C6C6D6FEEEC600UL,	// Char 087 (W)
	0xC6C66C38386CC600UL,	// Char 088 (X)
	0x6666663C18183C00UL,	// Char 089 (Y)
	0xFEC68C183266FE00UL,	// Char 090 (Z)
};

const uint64_t font_lowercase[] = {
	0x0000780C7CCC7600UL,	// Char 097 (a)
	0xE060607C6666DC00UL,	// Char 098 (b)
	0x00003C6660663C00UL,	// Char 099 (c)
	0x1C0C0C7CCCCC7600UL,	// Char 100 (d)
	0x00003C667E603C00UL,	// Char 101 (e)
	0x1C36307830307800UL,	// Char 102 (f)
	0x000076CCCC7C0CF8UL,	// Char 103 (g)
	0xE0606C766666E600UL,	// Char 104 (h)
	0x1800381818183C00UL,	// Char 105 (i)
	0x060006060666663CUL,	// Char 106 (j)
	0xE060666C786CE600UL,	// Char 107 (k)
	0x3818181818183C00UL,	// Char 108 (l)
	0x0000CCFEFED6C600UL,	// Char 109 (m)
	0x0000DC6666666600UL,	// Char 110 (n)
	0x00003C6666663C00UL,	// Char 111 (o)
	0x0000DC66667C60F0UL,	// Char 112 (p)
	0x000076CCCC7C0C1EUL,	// Char 113 (q)
	0x0000DC766660F000UL,	// Char 114 (r)
	0x00003E603C067C00UL,	// Char 115 (s)
	0x10307C3030341800UL,	// Char 116 (t)
	0x0000CCCCCCCC7600UL,	// Char 117 (u)
	0x00006666663C1800UL,	// Char 118 (v)
	0x0000C6D6FEFE6C00UL,	// Char 119 (w)
	0x0000C66C386CC600UL,	// Char 120 (x)
	0x00006666663E067CUL,	// Char 121 (y)
	0x00007E4C18327E00UL,	// Char 122 (z)
};

const uint64_t font_hebrew[] = {
	0x00C66676DCCCC600UL,	// Char 128 (א)
	0x00F80C0C0C0CFE00UL,	// Char 129 (ב)
	0x00380C0C1C34E600UL,	// Char 130 (ג)
	0x00FE0C0C0C0C0C00UL,	// Char 131 (ד)
	0x00FC0606C6C6C600UL,	// Char 132 (ה)
	0x0070181818181800UL,	// Char 133 (ו)
	0x003C18180C183000UL,	// Char 134 (ז)
	0x00FC66C6C6C6C600UL,	// Char 135 (ח)
	0x00CCD6D6C6CC7800UL,	// Char 136 (ט)
	0x00380C0C18000000UL,	// Char 137 (י)
	0x00FC06060C0C0C0EUL,	// Char 138 (ך)
	0x00FC06060606FC00UL,	// Char 139 (כ)
	0xC0FC06060C181800UL,	// Char 140 (ל)
	0x00FC66C6C6C6FE00UL,	// Char 141 (ם)
	0x00DC7666C6C6DE00UL,	// Char 142 (מ)
	0x00380C181818181CUL,	// Char 143 (ן)
	0x00380C0C0C0C7C00UL,	// Char 144 (נ)
	0x00FC66C6C6CC7800UL,	// Char 145 (ס)
	0x006666666636FC00UL,	// Char 146 (ע)
	0x00F84CCCEC0C0C0EUL,	// Char 147 (ף)
	0x00FC46C6E606FE00UL,	// Char 148 (פ)
	0x006666667C606070UL,	// Char 149 (ץ)
	0x0066361C0C067E00UL,	// Char 150 (צ)
	0x00FC06666C6E6060UL,	// Char 151 (ק)
	0x00FC060606060600UL,	// Char 152 (ר)
	0x00D6D6D6F6C67C00UL,	// Char 153 (ש)
	0x00FC666666E6E600UL,	// Char 154 (ת)
};

uint64_t led_glyph(char c) {
	if (c == ' ') 
		return 0;
	if (c >= '0' && c <= '9')
		return font_digits[c-'0'];
	if (c >= 'A' && c <= 'Z')
		return font_uppercase[c-'A'];
	if (c >= 'a' && c <= 'z')
		return font_lowercase[c-'a'];
	uint8_t uc = c;
	if (uc >= 0x80 && uc <= 0x9b)
		return font_hebrew[uc-0x80];

	return 0xA5A5A5A5A5A5A5A5ULL; // unknown glyph
}

void led_swipe(const struct device *led, uint64_t cur, uint64_t new,
	char direction, uint32_t delay)
{
	for (unsigned i = 0; i < 8; i++) {
		uint64_t old = cur;
		
		// calculate frame
		switch(direction) {
			case 'U':
				cur = (cur << 8) | (new >> 56);
				new <<= 8;
				break;
			case 'D':
				cur = (cur >> 8) | (new << 56);
				new >>= 8;
				break;
			case 'L':
				cur = ((cur << 1) & ~H_MASK) | ((new >> 7) & H_MASK);
				new <<= 1;
				break;
			case 'R':
				cur = ((cur & ~H_MASK) >> 1) | ((new & H_MASK) << 7);
				new >>= 1;
				break;
		}

		// flip modified LEDs
		uint64_t xor = old ^ cur;
		for (unsigned j = 0; j < 64; j++) {
			if ((xor >> j) & 1) {
				if ((cur >> j) & 1)
					led_on(led, POS_TO_LED(j));
				else
					led_off(led, POS_TO_LED(j));
			}
		}
		//printk("old=%016llx cur=%016llx xor=%016llx\n", old, cur, xor);

		k_msleep(delay);
	}
}

void boot_animation(const struct device *led)
{
	printk("boot animation started\n");

	uint64_t cur = 0;
	const char s1[] = "Hackeriot";

	for (const char *c = s1; *c; ++c) {
		uint64_t old = cur;
		cur = led_glyph(*c);
		led_swipe(led, old, cur, 'L', 10);
	}

	k_msleep(100);

	// האקריות
	const char s2[] = {0x84, 0x80, 0x97, 0x98, 0x89, 0x85, 0x9a, 0};
	for (const char *c = s2; *c; ++c) {
		uint64_t old = cur;
		cur = led_glyph(*c);
		led_swipe(led, old, cur, 'R', 10);
	}

	k_msleep(100);

	// clear
	led_swipe(led, cur, 0, 'R', 0);
}

void breath_thread_func(void*, void*, void*)
{
	const struct device *const led = DEVICE_DT_GET(LED_NODE);
	if ( ! device_is_ready(led)) {
		printk("[bt] LED device not ready\n");
		return;
	}
	// n=15; [int(.48+(1000*2/math.pi)*(math.asin((i+1)/n)-math.asin(i/n))) for i in range(n)][::-1]
	static const uint8_t delta[] = {
		234, 99, 77, 66, 59, 55, 52, 49, 47, 46, 44, 44, 43, 43, 42};
	while(1) {
		for (int i = 0; i < 15; i++) {
			led_set_brightness(led, 0, (15-i)*100/15);
			k_usleep(delta[i]*400);
		}
		for (int i = 0; i < 15; i++) {
			led_set_brightness(led, 0, i*100/15);
			k_usleep(delta[14-i]*400);
		}
	}
}