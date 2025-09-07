/*
 * Copyright (c) 2025 Benny Meisels <benny.meisels@gmail.com>
 *                    Rani Hod <rani.hod@gmail.com>
 *
 * SPDX-License-Identifier: Apache-2.0
 */


#include <zephyr/kernel.h>
#include <zephyr/sys/printk.h>

#include "buttons.h"
#include "led.h"
#include "screen.h"

enum language lang = LANG_HE;

#define H_MASK 0x0101010101010101ULL
#define UTF8_HEBREW_MSB 0xd7

#ifdef BREADBOARD
	// Adafruit's dual-colored HT16K33
	#define POS_TO_LED(x) ((x&7) | ((x&~7) << 1))
#else
	// board2025 monochromatic HT16K33, rotated
	#define POS_TO_LED(x) (((63-x)&7) | (((63-x)&~7) << 1))
#endif

static struct screen_data_t {
    uint64_t bitmap;
    uint64_t blink_fast_mask;
    uint64_t blink_slow_mask;
} screen_data;

static void screen_thread_func(void *, void *, void *)
{
    const struct device *const led = DEVICE_DT_GET(LED_NODE);
	if ( ! device_is_ready(led)) {
		printk("[%s] LED device not ready\n", __func__);
		return;
	}

    uint32_t tick = 0;
    static uint64_t current = 0; // all blank
    while(1) {
        // calculate LEDs to invert [TODO: mutex]
        uint64_t inv_mask = current ^ screen_data.bitmap;
        inv_mask &= ~(screen_data.blink_fast_mask | screen_data.blink_slow_mask);
        if (tick % (SCREEN_FPS/SCREEN_BLINK_FAST) == 0)
            inv_mask |= screen_data.blink_fast_mask;
        if (tick % (SCREEN_FPS/SCREEN_BLINK_SLOW) == 0)
            inv_mask |= screen_data.blink_slow_mask;

        if (inv_mask) {
            // invert LEDs
            for (unsigned j = 0; j < 64; j++) {
                if ((inv_mask >> j) & 1) {
                    unsigned pos = POS_TO_LED(j);
                    if ((current >> j) & 1)
                        led_off(led, pos);
                    else
                        led_on(led, pos);
                }
            }
            // update current LED bitmap
            current ^= inv_mask;

        }
        //printk("(%d) inv_mask=%016llx current=%016llx\n", tick, inv_mask, current);

        // sleep until next tick
        ++tick;
        k_msleep(1000 / SCREEN_FPS);
    }
}

K_THREAD_DEFINE(
    screen_tid, 500,    // name, stack_size
    screen_thread_func, // entry
    NULL, NULL, NULL,   // p1, p2, p3
    10, 0, 1);          // prio, options, delay


// blinkall functions
void screen_blinkall(enum blink_speed bs)
{
    const struct device *const led = DEVICE_DT_GET(LED_NODE);
    led_blink(led, 0, bs, bs);
}

// glyph constants and functions
static const uint64_t glyph_printable_ascii[] = {
	0x183C3C1818001800,	// Char 033 (!)
	0x6C6C6C0000000000,	// Char 034 (")
	0x6C6CFE6CFE6C6C00,	// Char 035 (#)
	0x183E603C067C1800,	// Char 036 ($)
	0x00C6CC183066C600,	// Char 037 (%)
	0x386C3876DCCC7600,	// Char 038 (&)
	0x3030600000000000,	// Char 039 (')
	0x0C18303030180C00,	// Char 040 (()
	0x30180C0C0C183000,	// Char 041 ())
	0x00663CFF3C660000,	// Char 042 (*)
	0x0018187E18180000,	// Char 043 (+)
	0x0000000000181830,	// Char 044 (,)
	0x0000007E00000000,	// Char 045 (-)
	0x0000000000181800,	// Char 046 (.)
	0x060C183060C08000,	// Char 047 (/)
	0x386CC6D6C66C3800,	// Char 048 (0)
	0x1838181818187E00,	// Char 049 (1)
	0x3C66061C30667E00,	// Char 050 (2)
	0x3C66061C06663C00,	// Char 051 (3)
	0x1C3C6CCCFE0C1E00,	// Char 052 (4)
	0x7E607C0606663C00,	// Char 053 (5)
	0x1C30607C66663C00,	// Char 054 (6)
	0x7E66060C18181800,	// Char 055 (7)
	0x3C66663C66663C00,	// Char 056 (8)
	0x3C66663E060C3800,	// Char 057 (9)
	0x0018180000181800,	// Char 058 (:)
	0x0018180000181830,	// Char 059 (;)
	0x0C18306030180C00,	// Char 060 (<)
	0x00007E00007E0000,	// Char 061 (=)
	0x30180C060C183000,	// Char 062 (>)
	0x3C66060C18001800,	// Char 063 (?)
	0x7CC6DEDEDEC07800,	// Char 064 (@)
	0x183C66667E666600,	// Char 065 (A)
	0xFC66667C6666FC00,	// Char 066 (B)
	0x3C66C0C0C0663C00,	// Char 067 (C)
	0xF86C6666666CF800,	// Char 068 (D)
	0xFE6268786862FE00,	// Char 069 (E)
	0xFE6268786860F000,	// Char 070 (F)
	0x3C66C0C0CE663E00,	// Char 071 (G)
	0x6666667E66666600,	// Char 072 (H)
	0x3C18181818183C00,	// Char 073 (I)
	0x1E0C0C0CCCCC7800,	// Char 074 (J)
	0xE6666C786C66E600,	// Char 075 (K)
	0xF06060606266FE00,	// Char 076 (L)
	0xC6EEFEFED6C6C600,	// Char 077 (M)
	0xC6E6F6DECEC6C600,	// Char 078 (N)
	0x7CC6C6C6C6C67C00,	// Char 079 (O)
	0xFC66667C6060F000,	// Char 080 (P)
	0x7CC6C6C6C6CE7C0E,	// Char 081 (Q)
	0xFC66667C6C66E600,	// Char 082 (R)
	0x3C6670380E663C00,	// Char 083 (S)
	0x7E5A181818183C00,	// Char 084 (T)
	0x6666666666667E00,	// Char 085 (U)
	0x66666666663C1800,	// Char 086 (V)
	0xC6C6C6D6FEEEC600,	// Char 087 (W)
	0xC6C66C38386CC600,	// Char 088 (X)
	0x6666663C18183C00,	// Char 089 (Y)
	0xFEC68C183266FE00,	// Char 090 (Z)
	0x3C30303030303C00,	// Char 091 ([)
	0xC06030180C060200,	// Char 092 (\)
	0x3C0C0C0C0C0C3C00,	// Char 093 (])
	0x10386CC600000000,	// Char 094 (^)
	0x00000000000000FF,	// Char 095 (_)
	0x3030180000000000,	// Char 096 (`)
	0x0000780C7CCC7600,	// Char 097 (a)
	0xE060607C6666DC00,	// Char 098 (b)
	0x00003C6660663C00,	// Char 099 (c)
	0x1C0C0C7CCCCC7600,	// Char 100 (d)
	0x00003C667E603C00,	// Char 101 (e)
	0x1C36307830307800,	// Char 102 (f)
	0x000076CCCC7C0CF8,	// Char 103 (g)
	0xE0606C766666E600,	// Char 104 (h)
	0x1800381818183C00,	// Char 105 (i)
	0x060006060666663C,	// Char 106 (j)
	0xE060666C786CE600,	// Char 107 (k)
	0x3818181818183C00,	// Char 108 (l)
	0x0000CCFEFED6C600,	// Char 109 (m)
	0x0000DC6666666600,	// Char 110 (n)
	0x00003C6666663C00,	// Char 111 (o)
	0x0000DC66667C60F0,	// Char 112 (p)
	0x000076CCCC7C0C1E,	// Char 113 (q)
	0x0000DC766660F000,	// Char 114 (r)
	0x00003E603C067C00,	// Char 115 (s)
	0x10307C3030341800,	// Char 116 (t)
	0x0000CCCCCCCC7600,	// Char 117 (u)
	0x00006666663C1800,	// Char 118 (v)
	0x0000C6D6FEFE6C00,	// Char 119 (w)
	0x0000C66C386CC600,	// Char 120 (x)
	0x00006666663E067C,	// Char 121 (y)
	0x00007E4C18327E00,	// Char 122 (z)
	0x0E18187018180E00,	// Char 123 ({)
	0x1818180018181800,	// Char 124 (|)
	0x7018180E18187000,	// Char 125 (})
	0x76DC000000000000,	// Char 126 (~)
};

static const uint64_t glyph_hebrew[] = {
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

uint64_t get_glyph(char ch)
{
	if (ch == ' ') 
		return 0;
	if (ch >= '!' && ch <= '~')
		return glyph_printable_ascii[ch - '!'];
	uint8_t uch = ch;
	if (uch >= 0x90 && uch <= 0xab)
		return glyph_hebrew[uch - 0x90];

	return 0xA55AA55AA55AA55AULL; // unknown
}

// pixel functions
void screen_mask_on(uint64_t mask)
{
    // TODO: mutex
    screen_data.bitmap |= mask;
    screen_data.blink_fast_mask &= ~mask;
    screen_data.blink_slow_mask &= ~mask;
}

void screen_mask_off(uint64_t mask)
{
    // TODO: mutex
    screen_data.bitmap &= ~mask;
    screen_data.blink_fast_mask &= ~mask;
    screen_data.blink_slow_mask &= ~mask;
}

void screen_mask_invert(uint64_t mask)
{
    // TODO: mutex
    screen_data.bitmap ^= mask;
    screen_data.blink_fast_mask &= ~mask;
    screen_data.blink_slow_mask &= ~mask;
}

void screen_mask_blink(uint64_t mask, bool fast)
{
    // TODO: mutex
    screen_data.bitmap &= ~mask;
    if (fast) {
        screen_data.blink_fast_mask |= mask;
        screen_data.blink_slow_mask &= ~mask;
    } else {
        screen_data.blink_fast_mask &= ~mask;
        screen_data.blink_slow_mask |= mask;
    }
}

// bitmap functions
void screen_set(uint64_t bitmap)
{
    // TODO: mutex
    screen_data.bitmap = bitmap;
    screen_data.blink_fast_mask = 0;
    screen_data.blink_slow_mask = 0;
}

char screen_swipe(uint64_t bitmap, char direction, 
    k_timeout_t pixel_delay, const char *buttons)
{
    uint64_t current = screen_data.bitmap;
    char btn = 0;

    unsigned glyph_width = 8; // TODO: variable
    for (unsigned i = 0; i < glyph_width && ! btn; i++) {
        switch(direction) {
            case 'U':
                current = (current << 8) | (bitmap >> 56);
                bitmap <<= 8;
                break;
            case 'D':
                current = (current >> 8) | (bitmap << 56);
                bitmap >>= 8;
                break;
            case 'L':
                current = ((current << 1) & ~H_MASK) | ((bitmap >> 7) & H_MASK);
                bitmap <<= 1;
                break;
            case 'R':
                current = ((current & ~H_MASK) >> 1) | ((bitmap & H_MASK) << 7);
                bitmap >>= 1;
                break;
            default:
                printk("[%s] unexpected dir=%c (%02x)\n", __func__, direction, direction);
        }
        screen_set(current); // note: this stops all blinks
        
        if (buttons && ! *buttons)
            k_sleep(pixel_delay);
        else
            btn = buttons_get(buttons, pixel_delay);
    }
    return btn;
}

// text functions
char screen_scroll_once(const char *text, char direction, 
    k_timeout_t pixel_delay, const char *buttons)
{
    char btn = 0;
    while(*text && ! btn) {
        char ch = *text++;
        if (ch == UTF8_HEBREW_MSB) ch = *text++;
        btn = screen_swipe(get_glyph(ch), direction, pixel_delay, buttons);
    }
    return btn;
}

char screen_scroll_infinite(const char *text, char direction, 
    k_timeout_t pixel_delay, const char *buttons)
{
    char btn;
    do {
        btn = screen_scroll_once(text, direction, pixel_delay, buttons);
        if ( ! btn) btn = screen_swipe(0, direction, pixel_delay, buttons);
    } while ( ! btn);
    
    return btn;
}
