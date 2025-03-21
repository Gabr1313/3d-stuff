#ifndef DL_NAME_COPY
#define DL_NAME_COPY ".dll_copy.so"
#endif

#include <sys/stat.h>
#include <sys/sendfile.h>
#include <dlfcn.h>
#include <fcntl.h>
#include <unistd.h>

typedef struct {
	void  *ptr;
	char  *name;
	time_t last_update;
} DLLStats;

#ifdef DEV
void (*game_update)(GameState*, Canvas*);
void game_update_stub(GameState*, Canvas*) {}

void update_dl(DLLStats* dl) {
	struct stat stats;
    if (stat(dl->name, &stats)) {
        dbg("File not found: %s", dl->name);
        return;
    }

	time_t last_update = stats.st_mtime;
	if (dl->last_update == last_update || stats.st_size == 0) {
		return;
	}
	dl->last_update = last_update;

	game_update = game_update_stub;

    if (dl->ptr) {
		dlclose(dl->ptr);
	}

	char *copied_name = DL_NAME_COPY;
	i32 src_fd = open(dl->name, O_RDONLY);
    if (src_fd == -1) {
        dbge("Could not open %s", dl->name);
		return;
    }
    i32 dst_fd = open(copied_name, O_WRONLY|O_CREAT|O_TRUNC, 00644); // NOTE(gabri): do not remove leading zeroes!
    if (dst_fd == -1) {
        dbge("Could not open %s", copied_name);
		close(src_fd);
		return;
    }

    if (sendfile(dst_fd, src_fd, NULL, (u64)stats.st_size) != stats.st_size) {
        dbge("Error coping file %s to file %s", dl->name, copied_name);
		return;
    }
	close(src_fd);
	close(dst_fd);

    dl->ptr = dlopen(copied_name, RTLD_LAZY);
    if (!dl->ptr) {
        dbge("Error loading library: %s\n", dlerror());
        return;
    }

    game_update = dlsym(dl->ptr, "game_update");
    if (!game_update) {
		game_update = game_update_stub;
        dbge("Error finding symbol: %s\n", dlerror());
        return ;
    }
	dbg("Dynamic Library reloaded: %s", dl->name);
}

#else
#include "../game.c"
#define update_dl(dl)
#endif
