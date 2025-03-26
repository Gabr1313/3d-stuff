#pragma once

#include <sys/stat.h>
#include <dlfcn.h>

typedef struct {
	char  *name;
	void  *ptr;
	time_t    last_update;
} DLStats;

void func_stub() {}

#define dl_load_func(dl, fn_name_dl, fn) _dl_load_func(dl, fn_name_dl, (void(**)())fn)

b8 _dl_load_func(DLStats *dl, char* fn_name_dl, void(**fn)()) {
    *fn = (void(*)())dlsym(dl->ptr, fn_name_dl);
    if (!*fn) {
		*fn = (void(*)())func_stub;
        err("Error finding symbol: %s\n", dlerror());
        return false;
    }
	return true;
}


// return values:
// - `0`: dl didn't change
// - `1`: dl changed
// - `2`: failed to load new dl
i32 dl_update(DLStats *dl) {
	struct stat stats;
    if (stat(dl->name, &stats)) {
        log("File not found: %s", dl->name);
        return 0;
    }

	time_t last_update = stats.st_mtime;
	if (dl->last_update == last_update || stats.st_size == 0) {
		return 0;
	}
	dl->last_update = last_update;

    if (dl->ptr) {
		dlclose(dl->ptr);
	}

    dl->ptr = dlopen(dl->name, RTLD_LAZY);
    if (!dl->ptr) {
        err("Error loading library: %s\n", dlerror());
		return -1;
    }
	return 1;
}
