#include "main.h"
#include <stdio.h>
#include <fcntl.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

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
        if (event.type == EV_KEY)
        {
            printf("Pressed %d", event.code);
            if (event.code == KEY_A && event.value == 0)
            {
                printf("pressed A");
                struct input_event sending_event;
                sending_event.code = KEY_0;
                sending_event.type = EV_KEY;
                sending_event.value = 1;
                write(fkeyboard_device, &sending_event, sizeof(struct input_event));

                sending_event.value = 0;
                write(fkeyboard_device, &sending_event, sizeof(struct input_event));
                sending_event.type = EV_SYN;
                sending_event.code = SYN_REPORT;
                sending_event.value = 0;
                write(fkeyboard_device, &sending_event, sizeof(struct input_event));
                continue;
            }
        }
    }
}
