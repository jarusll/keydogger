#include <linux/input-event-codes.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef KEYDOGGER_H
#define KEYDOGGER_H

#define READABLE_KEYS 48

// Errors
#define EOPEN 1  // Cannot open
#define EREAD 2  // Cannot read
#define EINVCH 3 // Invalid character
#define EINVC 4  // Invalid keycode
#define EINIT 5  // Error initializing
#define EADD 6   // Error adding
#define ESETUP 7
#define ECREATE 8
#define EWRITE 9
#define EENV 10
#define EPERM 11
#define EFORK 12
#define ELEAD 13
#define ECHDIR 14
#define ERENAM 15
#define EPGREP 16
#define ECMD 17
#define EPIPE 18
#define EUSAGE 19
#define ESTR 20
#define EUSER 21

#define RC_PATH "keydoggerrc"
#define UINPUT_PATH "/dev/uinput"
#define PID_PATH "/run/keydogger.pid"

static const size_t *key_codes[] = {
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_0,
    KEY_MINUS,
    KEY_EQUAL,
    KEY_Q,
    KEY_W,
    KEY_E,
    KEY_R,
    KEY_T,
    KEY_Y,
    KEY_U,
    KEY_I,
    KEY_O,
    KEY_P,
    KEY_LEFTBRACE,
    KEY_RIGHTBRACE,
    KEY_A,
    KEY_S,
    KEY_D,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_SEMICOLON,
    KEY_APOSTROPHE,
    KEY_GRAVE,
    KEY_BACKSLASH,
    KEY_Z,
    KEY_X,
    KEY_C,
    KEY_V,
    KEY_B,
    KEY_N,
    KEY_M,
    KEY_COMMA,
    KEY_DOT,
    KEY_SLASH,
    KEY_SPACE,
    KEY_LEFTSHIFT,
    KEY_RIGHTSHIFT,
};

static const char *char_codes[] = {
    '1',
    '2',
    '3',
    '4',
    '5',
    '6',
    '7',
    '8',
    '9',
    '0',
    '-',
    '=',
    'q',
    'w',
    'e',
    'r',
    't',
    'y',
    'u',
    'i',
    'o',
    'p',
    '[',
    ']',
    'a',
    's',
    'd',
    'f',
    'g',
    'h',
    'j',
    'k',
    'l',
    ';',
    '\'',
    '`',
    '\\',
    'z',
    'x',
    'c',
    'v',
    'b',
    'n',
    'm',
    ',',
    '.',
    '/',
    ' ',
    '\0',
    '\0',
};

static const char *shifted_char_codes[] = {
    '!',
    '@',
    '#',
    '%',
    '$',
    '^',
    '&',
    '*',
    '(',
    ')',
    '_',
    '+',
    'Q',
    'W',
    'E',
    'R',
    'T',
    'Y',
    'U',
    'I',
    'O',
    'P',
    '{',
    '}',
    'A',
    'S',
    'D',
    'F',
    'G',
    'H',
    'J',
    'K',
    'L',
    ':',
    '"',
    '~',
    '|',
    'Z',
    'X',
    'C',
    'V',
    'B',
    'N',
    'M',
    '<',
    '>',
    '?',
    ' ',
    '\0',
    '\0',
};

struct trie
{
    char character;
    struct trie *next[READABLE_KEYS];
    size_t keycode;
    bool is_leaf;
    struct trie *parent;
    char *expansion;
};

void cleanup_trie(trie);
void cleanup();
void read_from_rc();
bool check_priveleges();
bool valid_key_code(size_t code);
char get_char_from_keycode(size_t keycode);
// very unusual error, mismatch between def and impl
// inline void send_key_to_device(int keyboard_device, struct input_event event);
void send_backspace(int device_fd, size_t n);
void send_sync(int device_fd);
void init_trie(struct trie *trie, char character);
size_t get_position_from_char(char character);
size_t get_keycode_from_char(char character);
void push_trie(char *key, char *expansion);
void daemonize_keydogger();
void keydogger_daemon();
void init_virtual_device(int vkeyboard_device);

#endif
