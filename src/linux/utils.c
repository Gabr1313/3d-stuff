#pragma once

#include <stdio.h>
#define _err(format, ...)  fprintf(stderr, format "\n", ##__VA_ARGS__)
#define err(...)  _err("[ERROR] " __VA_ARGS__)

#ifdef DBG
#define _dbg(format, ...)  fprintf(stdout, format "\n", ##__VA_ARGS__)
#define dbg(...)  _dbg("[INFO] " __VA_ARGS__)

#define assert(cond, message, ...)                                                       \
    do {                                                                                 \
        if (!(cond)) {                                                                   \
            err("Assert failed (%s:%u): `%s` : %s", __FILE__, __LINE__, #cond, message); \
            *(volatile u8*)0 = 0;                                                        \
        }                                                                                \
    } while (0)

#else

#define assert(...) do {} while(0)
#define dbg(...)    do {} while(0)

#endif
