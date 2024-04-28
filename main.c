#include "main.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stddef.h>

// Errors
#define EOPEN 1
#define EREAD 2

static char *KEYBOARD_DEVICE = KEYBOARD_EVENT_PATH;
static struct hashtable Hashtable;

int main()
{
    int fkeyboard_device = open(KEYBOARD_DEVICE, O_RDWR | O_APPEND, NULL);
    if (fkeyboard_device < 0)
    {
        fprintf(STDERR_FILENO, "Error opening %s", KEYBOARD_DEVICE);
        exit(EOPEN);
    }

    for (size_t i = 0; i < sizeof(key_matrix) / sizeof(key_matrix[0]); i++)
    {
        printf("%d", sizeof(key_matrix[i]));
    }
    // hashtable_init();
    // start_expanding(&fkeyboard_device);
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

void start_expanding(int *fkeyboard_device)
{
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

struct hashitem get_null_hashitem()
{
    struct hashitem item;
    item.key = 0;
    item.value = '\0';
    return item;
}

void hashtable_init()
{
    for (int i = 0; i < sizeof(key_matrix) / sizeof(key_matrix[0]); i++)
    {
        if (key_matrix[i] != NULL)
        {
            add_to_hashtable(i, key_matrix[i][0]);
        }
    }
}

struct hashitem get(size_t key)
{
    for (int i = 0; i < 256; i++)
    {
        if (Hashtable.items[i].key == key)
        {
            return Hashtable.items[i];
        }
    }

    return get_null_hashitem();
};

struct hashitem get_by_value(char value)
{
    for (int i = 0; i < 256; i++)
    {
        if (Hashtable.items[i].value == value)
        {
            return Hashtable.items[i];
        }
    }

    return get_null_hashitem();
};

void add_to_hashtable(size_t key, char value)
{
    for (size_t i = 0; i < HASH_SIZE; i++)
    {
        if (Hashtable.items[i].key == key)
        {
            Hashtable.items[i].value = value;
            return;
        }
    }

    for (size_t i = 0; i < HASH_SIZE; i++)
    {
        if (Hashtable.items[i].key == 0)
        {
            Hashtable.items[i].key = key;
            Hashtable.items[i].value = value;
            Hashtable.len++;
            return;
        }
    }
}
