#ifndef __KC_H__
#define __KC_H__

struct konami_code_t {
	unsigned int state	: 4;	// number of matches so far
	unsigned int active	: 1;
};

extern struct konami_code_t konami_code;

#endif // __KC_H__
