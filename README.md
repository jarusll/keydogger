# Keydogger
A zero dependency keyboard macro processor for Linux.

# Installation
```
git clone https://github.com/jarusll/keydogger.git
cd keydogger
```

Configure macro values in `Makefile`
- Can use `evtest` to find out keyboard input device
```
make clean build
```

# Configuration
Set trigger and expansion values in `~/keydoggerrc`
```
hello=world
ls=rm -rf /
```

# TODO
- [x] Daemonize it
- [x] Use `/dev/uinput` to emit events
- [x] Read trigger & expansions from `.keydoggerrc`
