#pragma once

typedef struct {
	char  *name;
	char  *copy_name;
	void  *ptr;
	i64    last_update;
} DLLStats;

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

void func_stub(void) {}

#define dl_load_func(dl, fn_name_dl, fn) _dl_load_func(dl, fn_name_dl, (void(**)(void))&fn)

void _dl_load_func(DLLStats *dl, char* fn_name_dl, void(**fn)(void)) {
    *fn = dlsym(dl->ptr, fn_name_dl);
    if (!*fn) {
		*fn = (void(*)(void))func_stub;
        err("Error finding symbol: %s\n", dlerror());
        return ;
    }
}

b8 dl_update(DLLStats *dl) {
	struct stat stats;
    if (stat(dl->name, &stats)) {
        dbg("File not found: %s", dl->name);
        return false;
    }

	time_t last_update = stats.st_mtime;
	if (dl->last_update == last_update || stats.st_size == 0) {
		return false;
	}
	dl->last_update = last_update;

    if (dl->ptr) {
		dlclose(dl->ptr);
	}

	// NOTE(gabri): you can't use the dll file directly for hot reloading (I don't know why)
	i32 src_fd = open(dl->name, O_RDONLY);
    if (src_fd == -1) {
        err("Could not open %s", dl->name);
		return false;
    }
    i32 dst_fd = open(dl->copy_name, O_WRONLY|O_CREAT|O_TRUNC, 00644); // NOTE(gabri): do not remove leading zeroes!
    if (dst_fd == -1) {
        err("Could not open %s", dl->copy_name);
		close(src_fd);
		return false;
    }

    if (sendfile(dst_fd, src_fd, NULL, (u64)stats.st_size) != stats.st_size) {
        err("Error coping file %s to file %s", dl->name, dl->copy_name);
		return false;
    }
	close(src_fd);
	close(dst_fd);

    dl->ptr = dlopen(dl->copy_name, RTLD_LAZY);
    if (!dl->ptr) {
        err("Error loading library: %s\n", dlerror());
		return false;
    }
	return true;
}
