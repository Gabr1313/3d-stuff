#pragma once

// TODO: find a better name
#define err(...)  SDL_Log("[ERROR] " __VA_ARGS__)

#ifdef DBG

#define dbg(...)  SDL_Log("[INFO] " __VA_ARGS__)

#define assert(cond, message)                                             \
    do {                                                                  \
        _Pragma("GCC diagnostic push")                                    \
        _Pragma("GCC diagnostic ignored \"-Wanalyzer-null-dereference\"") \
        if (!(cond)) {                                                    \
            err("Assert failed: `%s` : %s", #cond, message);             \
            *(volatile u8*)0 = 0;                                         \
        }                                                                 \
        _Pragma("GCC diagnostic pop")                                     \
    } while (0)

#else

#define assert(cond, message) do {} while(0)
#define dbg(...) do {} while(0)

#endif
