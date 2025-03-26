#pragma once

#include <stdio.h>
#define _log(stream, format, ...)  fprintf(stream, format "\n", ##__VA_ARGS__)
#define err(...)  _log(stderr, "[ERROR] " __VA_ARGS__)


#ifdef DBG
#define log(...)  _log(stdout, "[INFO] " __VA_ARGS__)
#define dbg(...)  _log(stdout, "[DBG] " __VA_ARGS__)

#define assert(cond, message, ...)                                  \
    do {                                                            \
        if (!(cond)) {                                              \
            err("Assert failed (%s:%u): `%s` : " message, __FILE__, \
					__LINE__, #cond, ##__VA_ARGS__);                \
            *(volatile u8*)0 = 0;                                   \
        }                                                           \
    } while (0)

#else

#define assert(...) do {} while(0)
#define dbg(...)    do {} while(0)
#define log(...)    do {} while(0)

#endif

#define max(a, b) (a) > (b) ? (a) : (b)
#define min(a, b) (a) < (b) ? (a) : (b)
