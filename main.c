#include "main.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>

#if !defined(BUFFER_SIZE)
#define BUFFER_SIZE 1024
#endif

// Errors
#define EOPEN 1
#define EREAD 2

static char *KEYBOARD_DEVICE = KEYBOARD_EVENT_PATH;
static char BUFFER[BUFFER_SIZE];

int main()
{
    int fkeyboard_device = open(KEYBOARD_DEVICE, O_RDONLY, NULL);
    if (fkeyboard_device < 0)
    {
        exit(EOPEN);
    }

    struct input_event event;
    while (1)
    {
        int read_inputs = read(fkeyboard_device, &event, sizeof(struct input_event));
        if (read_inputs < 0)
        {
            exit(EREAD);
        }
        if (event.type == EV_KEY)
        {
            printf("Pressed %d", event.code);
        }
    }
}
