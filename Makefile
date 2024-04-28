KEYBOARD_EVENT_PATH=/dev/input/event2

dev: main.o
	gcc -g -o expanse main.o

main.o: main.h main.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -g -O0 -c main.c -o main.o -Wall

.PHONY: debug
debug: dev
	sudo gdb expanse

.PHONY: memcheck
memcheck: dev
	valgrind --tool=memcheck ./expanse
