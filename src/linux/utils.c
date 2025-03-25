#pragma once

#include <stdio.h>
#define _err(format, ...)  fprintf(stderr, format "\n", ##__VA_ARGS__)
#define err(...)  _err("[ERROR] " __VA_ARGS__)


#ifdef DBG
#define _log(format, ...)  fprintf(stdout, format "\n", ##__VA_ARGS__)
#define log(...)  _log("[INFO] " __VA_ARGS__)
#define dbg(...)  _log("[DBG] " __VA_ARGS__)

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
