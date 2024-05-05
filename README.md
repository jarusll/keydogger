# Keydogger
A zero dependency keyboard macro processor for Linux.

# Installation
```
git clone https://github.com/jarusll/keydogger.git
cd keydogger
```

Configure `KEYBOARD_EVENT_PATH` in `Makefile`. Can use `evtest` to find out keyboard input device.
Run
```
make clean build
```

# Configuration
Set trigger and expansion values in `~/keydoggerrc`
```
@hello=world
```

# Acknowledgements
- [emisilve86](https://github.com/emisilve86/Keylogger-Daemon-Linux) for their code.
- [Arjun](https://github.com/uhrjun) for the name `Keydogger`

# TODO
## v1
- [x] Daemonize it
- [x] Use `/dev/uinput` to emit events
- [x] Read trigger & expansions from `.keydoggerrc`
## v2
- [ ] On the fly toggle using magic keychords
- [ ] Unicode support
