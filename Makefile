CC = g++
W_FLAGS = -Wall -Wextra -Wconversion -Wshadow -Wstrict-overflow -Wfloat-equal \
	-Wformat=2 -Wstrict-aliasing -Wcast-align \
	-Wpointer-arith -Wundef -Wnull-dereference \
	-Wno-write-strings
MILD_W_FLAGS = $(W_FLAGS) -Wno-unused-variable -Wno-unused-parameter
EXTRA_W_FLAGS = $(W_FLAGS) -fanalyzer -Wno-analyzer-null-dereference
FAST_FLAGS = -O3 -march=native -ffast-math 
SLOW_FLAGS = -O0 -DFPS=30
DEBUG_FLAGS = -DDBG -ggdb -fsanitize=address,undefined
SDL_FLAGS = -lSDL3
LINK_FLAGS = -lm
DL_LOCK = build/lock_dll

release: build
	$(CC) -o build/release src/linux/first.cpp \
		$(FAST_FLAGS) \
		$(SDL_FLAGS) $(LINK_FLAGS)

fast: build
	touch $(DL_LOCK)
	$(CC) -o build/game.so src/game.cpp \
		$(FAST_FLAGS) -DDEV -shared -fpic \
		$(MILD_W_FLAGS) \
		$(LINK_FLAGS)
	rm $(DL_LOCK)
	$(CC) -o build/fast src/linux/first.cpp \
		-DDL_LOCK_FILE=\"$(DL_LOCK)\" \
		$(FAST_FLAGS) -DDEV \
		$(MILD_W_FLAGS) \
		$(SDL_FLAGS) $(LINK_FLAGS)

debug: build 
	touch $(DL_LOCK)
	$(CC) -o build/game.so src/game.cpp \
		$(SLOW_FLAGS) -DDEV -shared -fpic \
		$(DEBUG_FLAGS) \
		$(MILD_W_FLAGS)
	rm $(DL_LOCK)
	$(CC) -o build/debug src/linux/first.cpp \
		-DDL_LOCK_FILE=\"$(DL_LOCK)\" \
		$(SLOW_FLAGS) -DDEV \
		$(DEBUG_FLAGS) \
		$(MILD_W_FLAGS) \
		$(SDL_FLAGS) $(LINK_FLAGS)

analyzer: build 
	$(CC) -fsyntax-only -o build/debug src/linux/first.cpp \
		$(SLOW_FLAGS) \
		$(DEBUG_FLAGS) \
		$(EXTRA_W_FLAGS) \
		$(SDL_FLAGS) $(LINK_FLAGS)

build: 
	mkdir -p build

clean:
	rm -rf build

fedora41:
	sudo dnf install SDL3-devel

# Building from source
#
# docs:
# 	mkdir -p vendor
# 	git clone --depth=1 https://github.com/libsdl-org/sdlwiki.git vendor/sdlwiki
#
# sdl:
# 	mkdir -p vendor
# 	git clone --depth=1 https://github.com/libsdl-org/SDL vendor/SDL
# 	cd vendor/SDL && \
# 	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
# 	-DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF && \
# 	cmake --build build --config Release
#
# SDL_FLAGS_LOCALE = -Ivendor/SDL/include -Lvendor/SDL/build/ -Wl,-rpath,vendor/SDL/build -lSDL3
