# Configure start
KEYBOARD_EVENT_PATH=/dev/input/event2
# Configure end

DEPENDENCIES=wl-copy wl-paste
DAEMON_NAME=keydoggerd
PREFIX=/usr/local/bin/
CALLGRIND_FILE=benchmark.out

.PHONY: build
build: keydogger.h keydogger.c dependency-check
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -w keydogger.c -o keydogger

.PHONY: dependency-check
dependency-check:
	@if which $(DEPENDENCIES) >/dev/null 2>&1; then \
		echo "Dependency missing: wl-clipboard"; \
	else \
		echo "Dependency found: wl-clipboard"; \
	fi

.PHONY: dev
dev: keydogger.o
	touch keydoggerrc
	gcc -g -o keydogger keydogger.o

.PHONY: install
install: build
	mkdir -p $(PREFIX)
	mv keydogger $(PREFIX)

keydogger.o: keydogger.h keydogger.c dependency-check
	gcc -DKEYBOARD_EVENT_PATH=\"$(KEYBOARD_EVENT_PATH)\" -DDAEMON_NAME=\"$(DAEMON_NAME)\" -g -O0 -c keydogger.c -o keydogger.o -Wall  -Wextra -Wpedantic -Wsign-conversion -Wimplicit-function-declaration -Warray-bounds

.PHONY: debug
debug: dev
	sudo gdb keydogger

.PHONY: memcheck
memcheck: dev
	sudo valgrind --tool=memcheck ./keydogger debug

.PHONY: benchmark
benchmark: dev
	-rm callgrind*
	sudo valgrind --tool=callgrind --dump-instr=yes --simulate-cache=yes --collect-jumps=yes --callgrind-out-file=$(CALLGRIND_FILE) ./keydogger debug

.PHONY: viz
viz:
	kcachegrind ./$(CALLGRIND_FILE)

.PHONY: clean
clean:
	-rm *.o > /dev/null 2>&1 || true
	-rm keydogger > /dev/null 2>&1 || true
	-rm callgrind* > /dev/null 2>&1 || true
	-rm vgcore* > /dev/null 2>&1 || true
	-rm *.out > /dev/null 2>&1 || true
