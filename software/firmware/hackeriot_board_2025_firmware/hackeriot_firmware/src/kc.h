struct konami_code_t {
	unsigned state	: 4;	// number of matches so far
	unsigned active	: 1;
};

extern struct konami_code_t konami_code;
