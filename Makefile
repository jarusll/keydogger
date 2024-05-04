# change this
KEYBOARD_EVENT_PATH=/dev/input/event2
DAEMON_NAME=keydoggerd
PREFIX=/usr/local/bin/

dev: keydogger.o
	gcc -g -o keydogger keydogger.o

build: keydogger.h keydogger.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -w keydogger.c -o keydogger

install: build
	mkdir -p $(PREFIX)
	mv keydogger $(PREFIX)

keydogger.o: keydogger.h keydogger.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -g -O0 -c keydogger.c -o keydogger.o -Wall

.PHONY: debug
debug: dev
	sudo gdb keydogger

.PHONY: memcheck
memcheck: dev
	sudo valgrind --tool=memcheck ./keydogger

.PHONY: clean
clean:
	-rm *.o
	-rm keydogger
