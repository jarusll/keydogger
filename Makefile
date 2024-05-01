KEYBOARD_EVENT_PATH=/dev/input/event2
DAEMON_NAME=keyloggerd

dev: keydogger.o
	gcc -g -o keydoggerd keydogger.o

keydogger.o: keydogger.h keydogger.c
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -g -O0 -c keydogger.c -o keydogger.o -Wall

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
