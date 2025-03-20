W_FLAGS = -Wall -Wextra -Wconversion -Wshadow -Wstrict-overflow -Wfloat-equal \
	-Wformat=2 -Wstrict-aliasing -Wcast-align \
	-Wstrict-prototypes -Wpointer-arith -Wundef -Wnull-dereference
D_FLAGS = -ggdb -fsanitize=address,undefined
# TODO: remove -lm
L_FLAGS = -lm -Ivendor/SDL/include -Lvendor/SDL/build/ -lSDL3 -Wl,-rpath,vendor/SDL/build

release: build
	cc -o build/rel src/first-linux.c \
		-O2 \
		$(L_FLAGS)

debug: build 
	cc -o build/dbg src/first-linux.c \
		-O0 -DDEV \
		$(W_FLAGS) \
		$(D_FLAGS) \
		$(L_FLAGS)

analyzer: build 
	cc -o build/dbg src/first-linux.c \
		-O0 -DDEV \
		$(W_FLAGS) \
		-fanalyzer \
		$(D_FLAGS) \
		$(L_FLAGS)

build: 
	mkdir -p build

clean:
	rm -rf build


# TODO: use the SDL package: you don't need to `git clone`
# https://github.com/libsdl-org/SDL/blob/main/docs/README-cmake.md
sdl: vendor/SDL
	cd vendor/SDL && \
	cmake -S . -B build -DCMAKE_BUILD_TYPE=Release \
	-DSDL_SHARED=ON -DSDL_STATIC=OFF -DSDL_TEST_LIBRARY=OFF && \
	cmake --build build --config Release

vendor/SDL:
	git clone --depth=1 https://github.com/libsdl-org/SDL vendor/SDL

fedora41:
	sudo dnf install gcc git-core make cmake \
		alsa-lib-devel pulseaudio-libs-devel nas-devel pipewire-devel \
		libX11-devel libXext-devel libXrandr-devel libXcursor-devel libXfixes-devel \
		libXi-devel libXScrnSaver-devel dbus-devel ibus-devel \
		systemd-devel mesa-libGL-devel libxkbcommon-devel mesa-libGLES-devel \
		mesa-libEGL-devel vulkan-devel wayland-devel wayland-protocols-devel \
		libdrm-devel mesa-libgbm-devel libusb1-devel libdecor-devel \
		pipewire-jack-audio-connection-kit-devel liburing-devel glibc-static

# This does not work: Segmentation fault don't know why
# static: build
# 	gcc -o build/first src/first-linux.c \
# 		-Ivendor/SDL/include -Lvendor/SDL/build/ -l:libSDL3.a \
# 		-static -lSDL3 -lm -ldl -lpthread
