KEYBOARD_EVENT_PATH=/dev/input/event2

dev: main.o
	gcc -g -o expanse main.o

main.o: main.h main.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -g -O0 -c main.c -o main.o

.PHONY: debug
debug: dev
	sudo gdb expanse
