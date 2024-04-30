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
#include <linux/uinput.h>

// Errors
#define EOPEN 1  // Cannot open
#define EREAD 2  // Cannot read
#define EINVCH 3 // Invalid character
#define EINVC 4  // Invalid keycode
#define EINIT 5  // Error initializing
#define EADD 6   // Error adding
#define ESETUP 7
#define ECREATE 7
#define EWRITE 7

#define UINPUT_PATH "/dev/uinput"

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

void send_key_to_device(int keyboard_device, struct input_event event)
{
    int write_status = write(keyboard_device, &event, sizeof(event));
    if (write_status < 0)
    {
        fprintf(STDERR_FILENO, "Error writing to virtual device");
        exit(EWRITE);
    }
}

void send_backspace(int device_fd, size_t n)
{
    struct input_event event;
    event.code = KEY_BACKSPACE;
    event.type = EV_KEY;
    for (size_t i = 0; i < n; i++)
    {
        event.value = 1;
        send_key_to_device(device_fd, event);
        event.value = 0;
        send_key_to_device(device_fd, event);
    }
}

void send_sync(int device_fd)
{
    struct input_event event;
    event.type = EV_SYN;
    event.value = SYN_REPORT;
    send_key_to_device(device_fd, event);
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

void send_to_keyboard(int keyboard_device, char *string)
{
    size_t len = strlen(string);
    struct input_event event;
    event.type = EV_KEY;
    for (size_t i = 0; i < len; i++)
    {
        char character = string[i];
        event.code = get_keycode_from_char(character);
        event.value = 1;
        send_key_to_device(keyboard_device, event);
        event.value = 0;
        send_key_to_device(keyboard_device, event);
    }
}

void start_expanse(int keyboard_device, int vkeyboard_device)
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
                    send_backspace(vkeyboard_device, key_char_count);
                    send_to_keyboard(vkeyboard_device, current_trie->expansion);
                    send_sync(vkeyboard_device);
                    current_trie = TRIE;
                }
                continue;
            }
            current_trie = TRIE;
        }
    }
}

void init_virtual_device(int vkeyboard_device)
{
    struct uinput_setup usetup;

    int status;
    // setup as keyboard
    if ((status = ioctl(vkeyboard_device, UI_SET_EVBIT, EV_KEY)) < 0)
    {
        fprintf(STDERR_FILENO, "Error initializing virtual input");
        exit(EINIT);
    }
    // setup keys to emit
    if ((status = ioctl(vkeyboard_device, UI_SET_KEYBIT, KEY_BACKSPACE)) < 0)
    {
        fprintf(STDERR_FILENO, "Error adding key to virtual input : %d", KEY_BACKSPACE);
        exit(EADD);
    }
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        if ((status = ioctl(vkeyboard_device, UI_SET_KEYBIT, key_codes[i])) < 0)
        {
            fprintf(STDERR_FILENO, "Error adding key to virtual input : %d", key_codes[i]);
            exit(EADD);
        }
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 0x7961646176;
    usetup.id.product = 1999;
    strcpy(usetup.name, "keydogger");

    if ((status = ioctl(vkeyboard_device, UI_DEV_SETUP, &usetup)) < 0)
    {
        fprintf(STDERR_FILENO, "Error setting up virtual device");
        exit(ESETUP);
    }
    if ((status = ioctl(vkeyboard_device, UI_DEV_CREATE)) < 0)
    {
        fprintf(STDERR_FILENO, "Error creating up virtual device");
        exit(ECREATE);
    };
}

int main()
{
    int fkeyboard_device = open(KEYBOARD_DEVICE, O_RDWR | O_APPEND, NULL);

    if (fkeyboard_device < 0)
    {
        fprintf(STDERR_FILENO, "Error opening %s", KEYBOARD_DEVICE);
        exit(EOPEN);
    }
    int vkeyboard_device = open(UINPUT_PATH, O_WRONLY);
    ;
    if (open < 0)
    {
        fprintf(STDERR_FILENO, "Error reading from %s", UINPUT_PATH);
        exit(EOPEN);
    }

    TRIE = malloc(sizeof(struct trie));
    init_trie(TRIE, NULL);
    push_trie("12", "hello");
    init_virtual_device(vkeyboard_device);
    start_expanse(fkeyboard_device, vkeyboard_device);
}
