W_FLAGS = -Wall -Wextra -Wconversion -Wshadow -Wstrict-overflow -Wfloat-equal \
	-Wformat=2 -Wstrict-aliasing -Wcast-align \
	-Wstrict-prototypes -Wpointer-arith -Wundef -Wnull-dereference
MILD_W_FLAGS = $(W_FLAGS) -Wno-unused-variable -Wno-unused-parameter
EXTRA_W_FLAGS = $(W_FLAGS) -fanalyzer -Wno-analyzer-null-dereference
DEBUG_FLAGS = -DDBG -ggdb -fsanitize=address,undefined
SDL_FLAGS = -lSDL3
LINK_FLAGS = -lm

release: build
	gcc -o build/release src/linux/first.c \
		-O3 -march=native \
		$(SDL_FLAGS) $(LINK_FLAGS)

fast: build
	gcc -o build/game.so src/game.c \
		-O3 -march=native -DDEV -shared -fpic \
		$(LINK_FLAGS)
	gcc -o build/fast src/linux/first.c \
		-O3 -march=native -DDEV \
		$(SDL_FLAGS) $(LINK_FLAGS)

debug: build 
	gcc -o build/game.so src/game.c \
		-O0 -DDEV -shared -fpic \
		$(DEBUG_FLAGS) \
		$(MILD_W_FLAGS)
	gcc -o build/debug src/linux/first.c \
		-O0 -DDEV \
		$(DEBUG_FLAGS) \
		$(MILD_W_FLAGS) \
		$(SDL_FLAGS) $(LINK_FLAGS)

analyzer: build 
	gcc -o build/debug src/linux/first.c \
		-O0 -DDEV \
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
