#include "main.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>
#include <stdbool.h>

// Errors
#define EOPEN 1
#define EREAD 2
#define READABLE_KEYS 48

static char *KEYBOARD_DEVICE = KEYBOARD_EVENT_PATH;

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

int main()
{
    int fkeyboard_device = open(KEYBOARD_DEVICE, O_RDWR | O_APPEND, NULL);
    if (fkeyboard_device < 0)
    {
        fprintf(STDERR_FILENO, "Error opening %s", KEYBOARD_DEVICE);
        exit(EOPEN);
    }

    struct input_event event;
    while (1)
    {
        int read_inputs = read(fkeyboard_device, &event, sizeof(struct input_event));
        if (read_inputs < 0)
        {
            fprintf(STDERR_FILENO, "Error reading from %s", KEYBOARD_DEVICE);
            exit(EREAD);
        }
        if (event.type == EV_KEY && valid_key_code(event.code) && event.value == 1)
        {
            // printf("Code %d | Char %c\n", event.code, get_char_from_keycode(event.code));
            if (event.code == KEY_A)
            {
                send_backspace(fkeyboard_device, 1);
                event.code = KEY_0;
                event.type = EV_KEY;
                event.value = 1;
                write(fkeyboard_device, &event, sizeof(event));
                event.value = 0;
                write(fkeyboard_device, &event, sizeof(event));
                send_sync(fkeyboard_device);
            }
        }
    }
}
