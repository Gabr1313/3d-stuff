# Just a repo where I play with 3d stuff

> [!WARNING]  
> The README might not be up to date

## Compilation
For compiling this project, you need:
- to be on Linux
- `g++` (or `clang++` -> case change the Makefile)
- `SDL3` 
    - either download the **developer version** from your distro package repository
    - or compile it from [source](https://github.com/libsdl-org/SDL/tree/main)

## Input
- `W` move forward
- `A` move left
- `S` move backward
- `D` move right
- `E` move up
- `Q` move down

- `R` pause gameplay (camera still moves)
- `F` freeze gameplay (do not refresh)

- `X` exit
- `ESC` exit

- `MOUSE-LEFT` focus/unfocus the window
- `MOUSE-WHEEL` roll camera

You can always change them in [source code](src/linux/first.cpp).
