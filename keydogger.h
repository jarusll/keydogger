#include <linux/input-event-codes.h>
#include <stddef.h>
#include <stdbool.h>

#ifndef KEYDOGGER_H
#define KEYDOGGER_H

#define READABLE_KEYS 48

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
};

static const char *char_matrix[] = {
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
    '"',
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
void start_expanse(int keyboard_device, int vkeyboard_device);
void init_virtual_device(int vkeyboard_device);

#endif
