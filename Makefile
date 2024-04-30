KEYBOARD_EVENT_PATH=/dev/input/event2

dev: keydogger.o
	gcc -g -o expanse keydogger.o

keydogger.o: keydogger.h keydogger.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -g -O0 -c keydogger.c -o keydogger.o -Wall

.PHONY: debug
debug: dev
	sudo gdb expanse

.phony: memcheck
memcheck: dev
	sudo valgrind --tool=memcheck ./expanse
