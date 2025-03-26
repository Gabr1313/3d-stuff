#pragma once

#include <sys/mman.h>

#include "../types.h"

// 64-bit aligned
struct Arena {
	void *first;
	void *ptr;
	u64 cap;
};

#define arena_push_struct(arena, type)            (type *)arena_push(     (arena), sizeof(type))
#define arena_push_struct_zero(arena, type)       (type *)arena_push_zero((arena), sizeof(type))
#define arena_push_array(arena, type, count)      (type *)arena_push(     (arena), sizeof(type)*(count))
#define arena_push_array_zero(arena, type, count) (type *)arena_push_zero((arena), sizeof(type)*(count))

#define _align_down(what) (u64(what)         & ~7ul)
#define _align_up(what)   ((u64(what) + 7ul) & ~7ul)

// if `memory.first == -1` an error occured. Check `errno`
// `NULL` is a valid `address`: the OS decides
// Otherwise cosider the fact that on Linux you only have from 0x600000000000 - 0x7fffffffffff
Arena arena_new(u64 size, void *address) {
	Arena arena = {};
	arena.cap = size;
	int prot_flags = PROT_READ|PROT_WRITE;
	int map_flags = MAP_PRIVATE|MAP_ANONYMOUS|(address ? MAP_FIXED|MAP_FIXED_NOREPLACE : 0);
	arena.first = mmap(address, arena.cap, prot_flags, map_flags, 0, 0);
	arena.ptr = arena.first;
	return arena;
}

void arena_release(Arena *arena) {
	munmap(arena->first, arena->cap);
}

void* arena_push(Arena *arena, u64 bytes) {
	void *retval = arena->ptr;
	arena->ptr = (u8*)(arena->ptr) + bytes;
	arena->ptr = (void*)_align_up(arena->ptr);
	return retval;
}

void* arena_push_zero(Arena *arena, u64 bytes) {
	void *retval = arena_push(arena, bytes);
	for (u64 *p = (u64*)retval; (void*)p < arena->ptr; ++p) {
		*p = 0;
	}
	return retval;
}

void arena_pop(Arena *arena, u64 bytes) {
	arena->ptr = (u8*)(arena->ptr) - bytes;
	arena->ptr = (void*)_align_down(arena->ptr);
}

void arena_clear(Arena *arena) {
	arena->ptr = arena->first;
}
