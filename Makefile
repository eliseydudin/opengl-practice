C = /opt/homebrew/opt/llvm@18/bin/clang

main: main.c
	$(C) $< -o $@ $(shell pkg-config --cflags --libs sdl2 assimp) -framework OpenGL -O3