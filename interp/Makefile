all: build example

build:
	gcc -std=c2x -O2 -g -Wall -Wextra -I../runtime main.c ../runtime/runtime.c ../runtime/gc.c -o lama_interp

example:
	./lama_interp tests/regression/test001.bc < tests/regression/test001.input