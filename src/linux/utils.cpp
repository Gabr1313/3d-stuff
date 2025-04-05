#pragma once

#include <stdio.h>

#include "../types.h"

#define _log(stream, format, ...)  fprintf(stream, format "\n", ##__VA_ARGS__)
#define err(message, ...)  _log(stderr, "[ERROR] (%s:%u) " message, __FILE__, __LINE__, ##__VA_ARGS__)
#define log(...)  _log(stdout, "[INFO] " __VA_ARGS__)

#ifdef DBG
#define dbg(...)  _log(stdout, "[DEBUG] " __VA_ARGS__)

#define assert(cond, message, ...)                                      \
    do {                                                                \
        if (!(cond)) {                                                  \
            err("Assert failed `%s` : " message, #cond, ##__VA_ARGS__); \
			exit(1);                                                    \
        }                                                               \
    } while (0)

#else

#define assert(...) do {} while(0)
#define dbg(...)    do {} while(0)

#endif

#define max(a, b) ((a) > (b) ? (a) : (b))
#define min(a, b) ((a) < (b) ? (a) : (b))

static inline f32 clamp(f32 a, f32 mn, f32 mx) {
	return max(min(a,mx), mn);
}

static inline f32 mix(f32 a, f32 b, f32 k) {
	return (1.0f-k)*a + k*b;
}
