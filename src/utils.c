#ifdef DEV
#define assert(cond, message)                                             \
    do {                                                                  \
        _Pragma("GCC diagnostic push")                                    \
        _Pragma("GCC diagnostic ignored \"-Wanalyzer-null-dereference\"") \
        if (!(cond)) {                                                    \
            SDL_Log("Assert failed: `%s` : %s", #cond, message);          \
            *(volatile u8*)0 = 0;                                         \
        }                                                                 \
        _Pragma("GCC diagnostic pop")                                     \
    } while (0)
#else
#define assert(cond, message) do {} while(0)
#endif
