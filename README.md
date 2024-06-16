# Keydogger
Keylogger but funnier

Keydogger is a tiny text expander written in C

## Prerequisites
- `wl-clipboard`
- `gcc`
- `make`

```bash
git clone https://github.com/jarusll/keydogger.git
cd keydogger
```

## Configuration

Configure `KEYBOARD_EVENT_PATH` in `Makefile`.
It should be of the pattern `/dev/input/eventX`. X will be different for your system. You can use `evtest` to find out your keyboard device path.

## Installation

To install, run the following as sudo
```bash
make clean install
```

## Testing
You can test out sample expansions from the local `keydoggerrc` file by
```bash
make clean build
sudo ./keydogger debug
```

## Usage

### Configuring expansions
Set trigger and expansion values in `~/keydoggerrc`
```
@hello=Hello, World!
@program=Keydogger
:love:=❤️
@gg=Good game folks, see you tomorrow
@@=jarusll@the-mail.net
:ai=explain in short and simple terms
@install=sudo package-manager install -y
@update=sudo package-manager update -y
@restart=sudo keydogger stop && sudo keydogger start
@backup=rsync -avz ~/important_files/ jarusll@homeserver:backups/
```

### Start the daemon
```bash
sudo keydogger start
```

### Query the daemon status
```bash
sudo keydogger status
```

### Stop the daemon
```bash
sudo keydogger stop
```

## Acknowledgements
- [emisilve86](https://github.com/emisilve86/Keylogger-Daemon-Linux) for their code
- [Espanso](https://github.com/federico-terzi/espanso) for the `1000` microseconds of delay between events
- [ydotool](https://github.com/ReimuNotMoe/ydotool) for direct mapping of `character code` to `Linux key code`

## TODO
### v1
- [x] Daemonize it
- [x] Use `/dev/uinput` to emit events
- [x] Read trigger & expansions from `.keydoggerrc`
### v2
- [x] Unicode support
