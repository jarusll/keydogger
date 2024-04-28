#include "main.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>

#if !defined(BUFFER_SIZE)
#define BUFFER_SIZE 1024
#endif

// Errors
#define EOPEN 1
#define EREAD 2

static char *KEYBOARD_DEVICE = KEYBOARD_EVENT_PATH;
static struct trie Trie;

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
        if (event.type == EV_KEY && key_matrix[event.code] != NULL)
        {
            printf("Code %d | MatrixCode %s\n", event.code, key_matrix[event.code]);
        }
    }
}

void make_global_trie(int count, ...)
{
    va_list args;
    va_start(args, count);
    for (int i = 0; i < count; i++)
    {
        char *string = va_arg(args, char *);
        add_to_global_trie(string);
    }
    va_end(args);
}

void add_to_global_trie(char *str)
{
    size_t length = strlen(str);

    for (size_t i = 0; i < length; i++)
    {
        char character = str[i];
    }
}
