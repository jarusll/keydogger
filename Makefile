# change this
KEYBOARD_EVENT_PATH=/dev/input/event2
DAEMON_NAME=keydoggerd
PREFIX=/usr/local/

dev: keydoggerd.o
	gcc -g -o keydoggerd keydoggerd.o

build: keydoggerd.h keydoggerd.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -w keydoggerd.c -o keydoggerd
	mv keydoggerd $(PREFIX)

keydoggerd.o: keydoggerd.h keydoggerd.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -g -O0 -c keydoggerd.c -o keydoggerd.o -Wall

.PHONY: debug
debug: dev
	sudo gdb keydoggerd

.PHONY: memcheck
memcheck: dev
	sudo valgrind --tool=memcheck ./keydoggerd

.PHONY: clean
clean:
	-rm *.o
	-rm keydoggerd
