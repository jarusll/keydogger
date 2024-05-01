# Keydoggerd
A zero dependency keyboard macro processor for Linux.

# Installation
Configure macro values in `Makefile`
```
make clean build
```

# Configuration
Set trigger and expansion values in `~/keydoggerrc`
```
hello=world
...
```

# TODO
- [x] Daemonize it
- [x] Use `/dev/uinput` to emit events
- [x] Read trigger & expansions from `.keydoggerrc`
