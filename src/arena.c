#pragma once

#include <sys/mman.h>

// push aligned 64-bit
typedef struct {
	void *first;
	void *ptr;
	u64 cap;
} Arena;

#define arena_push_struct(arena, type)            (type *)arena_push(     (arena), sizeof(type))
#define arena_push_struct_zero(arena, type)       (type *)arena_push_zero((arena), sizeof(type))
#define arena_push_array(arena, type, count)      (type *)arena_push(     (arena), sizeof(type)*(count))
#define arena_push_array_zero(arena, type, count) (type *)arena_push_zero((arena), sizeof(type)*(count))

#define _align_up(what) (((u64)(what) + 7ul) & ~7ul);
#define _align_down(what) ((u64)(what) & ~7ul);

// `NULL` is a valid `address`: the OS decides
// if `memory.first == -1` an error occured. Check `errno`
Arena arena_new(void* address, u64 size) {
	Arena arena = {0};
	arena.cap = size;
	arena.first = mmap(address, arena.cap,
		PROT_READ|PROT_WRITE,
		MAP_PRIVATE|MAP_ANONYMOUS|MAP_FIXED|MAP_FIXED_NOREPLACE,
		0, 0);
	arena.ptr = arena.first;
	return arena;
}

void arena_release(Arena *arena) {
	munmap(arena->first, arena->cap);
}

void* arena_push(Arena *arena, u64 bytes) {
	u64 *retval = arena->ptr;
	arena->ptr += bytes;
	arena->ptr = (void*)_align_up(arena->ptr);
	return retval;
}

void* arena_push_zero(Arena *arena, u64 bytes) {
	void *retval = arena_push(arena, bytes);
	for (u64 *p = retval; (void*)p < arena->ptr; ++p) {
		*p = 0;
	}
	return retval;
}

void arena_pop(Arena *arena, u64 bytes) {
	arena->ptr -= bytes;
	arena->ptr = (void*)_align_down(arena->ptr);
}

void arena_clear(Arena *arena) {
	arena->ptr = arena->first;
}
