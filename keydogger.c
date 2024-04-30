#include "keydogger.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>

// Errors
#define EOPEN 1  // Cannot open
#define EREAD 2  // Cannot read
#define EINVCH 3 // Invalid character
#define EINVC 4  // Invalid keycode

static char *KEYBOARD_DEVICE = KEYBOARD_EVENT_PATH;
static struct trie *TRIE = NULL;

bool valid_key_code(size_t code)
{
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        if (key_codes[i] == code)
            return true;
    }
    return false;
}

char get_char_from_keycode(size_t keycode)
{
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        if (key_codes[i] == keycode)
        {
            return char_matrix[i];
        }
    }
    exit(EINVC);
}

void send_backspace(int *device_fd, size_t n)
{
    struct input_event event;
    event.code = KEY_BACKSPACE;
    event.type = EV_KEY;
    for (size_t i = 0; i < n; i++)
    {
        event.value = 1;
        write(device_fd, &event, sizeof(event));
        event.value = 0;
        write(device_fd, &event, sizeof(event));
    }
}

void send_sync(int *device_fd)
{
    struct input_event event;
    event.type = EV_SYN;
    event.value = SYN_REPORT;
    write(device_fd, &event, sizeof(event));
}

void init_trie(struct trie *trie, char character)
{
    trie->character = '\0';
    trie->parent = NULL;
    if (character != NULL)
    {
        trie->character = character;
        trie->keycode = get_keycode_from_char(character);
    }
    trie->is_leaf = false;
    trie->expansion = NULL;
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        trie->next[i] = NULL;
    }
}

size_t get_position_from_char(char character)
{
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        if (character == char_matrix[i])
        {
            return i;
        }
    }
    exit(EINVCH);
}

size_t get_keycode_from_char(char character)
{
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        if (character == char_matrix[i])
        {
            return key_codes[i];
        }
    }
    exit(EINVCH);
}

void push_trie(char *key, char *expansion)
{
    struct trie *current_trie = TRIE;
    for (size_t i = 0; i < strlen(key); i++)
    {
        char character = key[i];
        size_t position = get_position_from_char(character);
        if (current_trie->next[position] == NULL)
        {
            current_trie->next[position] = malloc(sizeof(struct trie));
            init_trie(current_trie->next[position], character);
            current_trie->next[position]->parent = current_trie;
        }
        current_trie = current_trie->next[position];
    }
    current_trie->is_leaf = true;
    current_trie->expansion = malloc(strlen(expansion) + 1);
    strcpy(current_trie->expansion, expansion);
}

void send_to_keyboard(int *keyboard_device, char *string){
    size_t len = strlen(string);
    struct input_event event;
    event.type = EV_KEY;
    for (size_t i = 0; i < len; i++){
        char character = string[i];
        event.code = get_keycode_from_char(character);
        event.value = 1;
        write(keyboard_device, &event, sizeof(struct input_event));
        event.value = 0;
        write(keyboard_device, &event, sizeof(struct input_event));
    }
}

void start_expanse(int *keyboard_device)
{
    struct input_event event;
    struct trie *current_trie = TRIE;
    while (1)
    {
        int read_inputs = read(keyboard_device, &event, sizeof(struct input_event));
        if (read_inputs < 0)
        {
            fprintf(STDERR_FILENO, "Error reading from %s", KEYBOARD_DEVICE);
            exit(EREAD);
        }
        if (event.type == EV_KEY && valid_key_code(event.code) && event.value == 1)
        {
            char character = get_char_from_keycode(event.code);
            size_t position = get_position_from_char(character);
            if (current_trie->next[position] != NULL)
            {
                current_trie = current_trie->next[position];
                if (current_trie->is_leaf)
                {
                    // printf("Expansion %s\n", current_trie->expansion);
                    size_t key_char_count = 0;
                    struct trie *cursor = current_trie;
                    while (cursor->character != NULL)
                    {
                        key_char_count++;
                        cursor = cursor->parent;
                    }
                    // printf("Count %d\n", key_char_count);
                    send_backspace(keyboard_device, key_char_count);
                    send_to_keyboard(keyboard_device, current_trie->expansion);
                    send_sync(keyboard_device);
                    current_trie = TRIE;
                }
                continue;
            }
            current_trie = TRIE;
            // printf("Code %d | Char %c\n", event.code, get_char_from_keycode(event.code));
            // if (event.code == KEY_A)
            // {
            //     send_backspace(keyboard_device, 1);
            //     event.code = KEY_0;
            //     event.type = EV_KEY;
            //     event.value = 1;
            //     write(keyboard_device, &event, sizeof(event));
            //     event.value = 0;
            //     write(keyboard_device, &event, sizeof(event));
            //     send_sync(keyboard_device);
            // }
        }
    }
}

int main()
{
    int fkeyboard_device = open(KEYBOARD_DEVICE, O_RDWR | O_APPEND, NULL);
    if (fkeyboard_device < 0)
    {
        fprintf(STDERR_FILENO, "Error opening %s", KEYBOARD_DEVICE);
        exit(EOPEN);
    }
    TRIE = malloc(sizeof(struct trie));
    init_trie(TRIE, NULL);
    push_trie("12", "hello");
    start_expanse(fkeyboard_device);
}
