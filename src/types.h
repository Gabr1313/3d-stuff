#pragma once

#include <stdint.h>

typedef int8_t  i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;

typedef uint8_t  u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float  f32;
typedef double f64;

typedef uint8_t  b8;
typedef uint16_t b16;
typedef uint32_t b32;
typedef uint64_t b64;

typedef struct {
	b8 running;
} GameState;

#define kilo(x) ((x)*1024)
#define mega(x) (kilo(x)*1024)
#define giga(x) (mega(x)*1024)
