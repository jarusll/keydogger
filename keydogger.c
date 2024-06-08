#include <stdio.h>
#include <fcntl.h>
#include <linux/uinput.h>
#include <linux/input.h>
#include <unistd.h>
#include <stdlib.h>
#include <stddef.h>
#include <stdbool.h>
#include <string.h>
#include <signal.h>
#include <sys/prctl.h>
#include <sys/stat.h>
#include <wchar.h>
#include <locale.h>
#include <iconv.h>
#include <ctype.h>

#define UTF8_SEQUENCE_MAXLEN 6
#include "keydogger.h"

extern char **environ;
static char *KEYBOARD_DEVICE = KEYBOARD_EVENT_PATH;
static char *DAEMON = DAEMON_NAME;
static struct trie *TRIE = NULL;

char *DEBUG_RC_PATH = "./keydoggerrc";

static int fkeyboard_device;
static int vkeyboard_device;

// Direct mapping of char codes to linux key codes
static int char_codes[] = {
    // 0 -> 33
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    -1,
    KEY_SPACE,
    KEY_1 | FLAG_UPPERCASE,
    KEY_APOSTROPHE | FLAG_UPPERCASE,
    KEY_3 | FLAG_UPPERCASE,
    KEY_4 | FLAG_UPPERCASE,
    KEY_5 | FLAG_UPPERCASE,
    KEY_7 | FLAG_UPPERCASE,
    KEY_APOSTROPHE,
    KEY_9 | FLAG_UPPERCASE,
    KEY_0 | FLAG_UPPERCASE,
    KEY_8 | FLAG_UPPERCASE,
    KEY_EQUAL | FLAG_UPPERCASE,
    KEY_COMMA,
    KEY_MINUS,
    KEY_DOT,
    KEY_SLASH,
    KEY_0,
    KEY_1,
    KEY_2,
    KEY_3,
    KEY_4,
    KEY_5,
    KEY_6,
    KEY_7,
    KEY_8,
    KEY_9,
    KEY_SEMICOLON | FLAG_UPPERCASE,
    KEY_SEMICOLON,
    KEY_COMMA | FLAG_UPPERCASE,
    KEY_EQUAL,
    KEY_DOT | FLAG_UPPERCASE,
    KEY_SLASH | FLAG_UPPERCASE,
    KEY_2 | FLAG_UPPERCASE,
    KEY_A | FLAG_UPPERCASE,
    KEY_B | FLAG_UPPERCASE,
    KEY_C | FLAG_UPPERCASE,
    KEY_D | FLAG_UPPERCASE,
    KEY_E | FLAG_UPPERCASE,
    KEY_F | FLAG_UPPERCASE,
    KEY_G | FLAG_UPPERCASE,
    KEY_H | FLAG_UPPERCASE,
    KEY_I | FLAG_UPPERCASE,
    KEY_J | FLAG_UPPERCASE,
    KEY_K | FLAG_UPPERCASE,
    KEY_L | FLAG_UPPERCASE,
    KEY_M | FLAG_UPPERCASE,
    KEY_N | FLAG_UPPERCASE,
    KEY_O | FLAG_UPPERCASE,
    KEY_P | FLAG_UPPERCASE,
    KEY_Q | FLAG_UPPERCASE,
    KEY_R | FLAG_UPPERCASE,
    KEY_S | FLAG_UPPERCASE,
    KEY_T | FLAG_UPPERCASE,
    KEY_U | FLAG_UPPERCASE,
    KEY_V | FLAG_UPPERCASE,
    KEY_W | FLAG_UPPERCASE,
    KEY_X | FLAG_UPPERCASE,
    KEY_Y | FLAG_UPPERCASE,
    KEY_Z | FLAG_UPPERCASE,
    KEY_LEFTBRACE | FLAG_UPPERCASE,
    KEY_BACKSLASH,
    KEY_RIGHTBRACE | FLAG_UPPERCASE,
    KEY_6 | FLAG_UPPERCASE,
    KEY_MINUS | FLAG_UPPERCASE,
    KEY_GRAVE,
    KEY_A,
    KEY_B,
    KEY_C,
    KEY_D,
    KEY_E,
    KEY_F,
    KEY_G,
    KEY_H,
    KEY_I,
    KEY_J,
    KEY_K,
    KEY_L,
    KEY_M,
    KEY_N,
    KEY_O,
    KEY_P,
    KEY_Q,
    KEY_R,
    KEY_S,
    KEY_T,
    KEY_U,
    KEY_V,
    KEY_W,
    KEY_X,
    KEY_Y,
    KEY_Z,
    KEY_LEFTBRACE,
    KEY_BACKSLASH | FLAG_UPPERCASE,
    KEY_RIGHTBRACE,
    KEY_GRAVE | FLAG_UPPERCASE,
};

void set_environment()
{
    if (getenv("WAYLAND_DISPLAY") == NULL)
    {
        setenv("WAYLAND_DISPLAY", "wayland-0", 0);
    }
    if (getenv("XDG_RUNTIME_DIR") == NULL)
    {
        char *uid = getenv("SUDO_UID");
        if (uid == NULL)
        {
            printf("Error getting current users uid");
            exit(EUID);
        }
        char xdg_runtime_path[100];
        snprintf(xdg_runtime_path, 100, "/run/user/%s", uid);
        setenv("XDG_RUNTIME_DIR", xdg_runtime_path, 0);
    }
}

void cleanup_trie(struct trie *trie)
{
    if (trie == NULL)
    {
        return;
    }
    else
    {
        free(trie->expansion);
        for (size_t i = 0; i < READABLE_KEYS; i++)
        {
            cleanup_trie(trie->next[i]);
        }
        free(trie);
    }
}

void cleanup()
{
    close(fkeyboard_device);
    ioctl(vkeyboard_device, UI_DEV_DESTROY);
    close(vkeyboard_device);

    cleanup_trie(TRIE);
}

void wide_to_utf8(wchar_t *input, char *output)
{
    iconv_t cd = iconv_open("UTF-8", "WCHAR_T");
    size_t wcs_len = (wcslen(input) + 1) * sizeof(wchar_t);
    size_t utf8_len = (wcs_len + 1) * UTF8_SEQUENCE_MAXLEN;
    char *utf8_buffer = malloc(utf8_len + 1);
    char **inbuf = (char **)&input;
    char **outbuf = (char **)&output;
    utf8_buffer[utf8_len] = '\0';
    size_t ret = iconv(cd, inbuf, &wcs_len, outbuf, &utf8_len);
    if (ret == (size_t)-1)
    {
        wprintf(L"Error converting wchar string to utf8 - %ls\n", input);
        exit(ECVRT);
    }
}

void read_from_rc(char *path)
{
    char *user = getlogin();
    if (user == NULL)
    {
        printf("Error getting current user");
        exit(EUSER);
    }
    char rc_file_path[256];
    if (snprintf(rc_file_path, 256, "/home/%s/%s", user, RC_PATH) < 0)
    {
        printf("Error constructing config path %s\n", RC_PATH);
        exit(ESTR);
    }

    TRIE = malloc(sizeof(struct trie));
    init_trie(TRIE, NULL);
    if (path != NULL)
    {
        strcpy(rc_file_path, path);
    }
    FILE *rc_file = fopen(rc_file_path, "r");
    if (rc_file == NULL)
    {
        printf("Error opening %s\n", rc_file_path);
        exit(EOPEN);
    }

    wchar_t line[256];
    while (fgetws(line, sizeof(line), rc_file) != NULL)
    {
        // Tokenize to get key=value
        wchar_t *state = NULL;
        wchar_t *key = wcstok(line, L"=", &state);
        if (key)
        {
            wchar_t *value = wcstok(NULL, L"", &state);
            if (value)
            {
                wchar_t *newline = wcschr(value, L'\n');
                if (newline)
                {
                    *newline = L'\0';
                }
                // Convert wide strings to utf8
                size_t wcs_key_len = (wcslen(key) + 1) * sizeof(wchar_t);
                size_t utf8_key_len = (wcs_key_len + 1) * UTF8_SEQUENCE_MAXLEN;
                char *utf_key = malloc(utf8_key_len);
                size_t wcs_value_len = (wcslen(value) + 1) * sizeof(wchar_t);
                size_t utf8_value_len = (wcs_value_len + 1) * UTF8_SEQUENCE_MAXLEN;
                char *utf_value = malloc(utf8_value_len);
                wide_to_utf8(key, utf_key);
                wide_to_utf8(value, utf_value);
                push_trie(utf_key, utf_value);
            }
        }
    }
    fclose(rc_file);
}

bool valid_key_code(size_t code)
{
    // 2 -> 13 = 1 -> =
    if (code >= KEY_1 && code <= KEY_EQUAL)
        return true;
    // 16 -> 27 = q -> ]
    if (code >= KEY_Q && code <= KEY_RIGHTBRACE)
        return true;
    // 30 -> 40, 41, 42 = a -> ', `, leftshift
    if (code >= KEY_A && code <= KEY_LEFTSHIFT)
        return true;
    // 43 -> 54 = backslash -> rightshift
    if (code >= KEY_BACKSLASH && code <= KEY_RIGHTSHIFT)
        return true;
    // 57 -> space
    if (code == KEY_SPACE)
        return true;
    return false;
}

struct key get_key_from_char(char character)
{
    struct key key = {0};
    key.character = character;
    key.is_shifted = false;
    int position = (int)character - 1;
    int keycode = char_codes[position];
    if (keycode & FLAG_UPPERCASE)
    {
        key.is_shifted = true;
        // subtract the flag to just get the keycode
        keycode &= ~FLAG_UPPERCASE;
    }
    key.keycode = keycode;
    key.position = position;
    return key;
}

// TODO: make a hashtable or direct map event codes to char positions
int get_position_from_event_code(size_t event_code)
{
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        if (char_codes[i] == (int)event_code)
            return i;
    }
    exit(EINVC);
}

void send_key_to_device(int keyboard_device, struct input_event event)
{
    int write_status = write(keyboard_device, &event, sizeof(event));
    if (write_status < 0)
    {
        printf("Error writing to virtual device\n");
        exit(EWRITE);
    }
}

void send_backspace(int device_fd, size_t n)
{
    struct input_event event = {0};
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
    struct input_event event = {0};
    event.type = EV_SYN;
    event.value = SYN_REPORT;
    send_key_to_device(device_fd, event);
}

void send_shift_down()
{
    struct input_event event = {0};
    event.type = EV_KEY;
    event.code = KEY_LEFTSHIFT;
    event.value = 1;
    send_key_to_device(vkeyboard_device, event);
}

void send_shift_up()
{
    struct input_event event = {0};
    event.type = EV_KEY;
    event.code = KEY_LEFTSHIFT;
    event.value = 0;
    send_key_to_device(vkeyboard_device, event);
}

void send_to_keyboard(int device_fd, char *string)
{
    bool is_expansion_ascii = true;
    for (size_t i = 0; i < strlen(string); i++)
    {
        if (!isascii(string[i]))
            is_expansion_ascii = false;
    }
    if (is_expansion_ascii)
    {
        struct input_event event = {0};
        event.type = EV_KEY;
        for (size_t i = 0; i < strlen(string); i++)
        {
            struct key key = get_key_from_char(string[i]);
            event.code = key.keycode;
            event.value = 1;
if (key.is_shifted)
                send_shift_down();
            send_key_to_device(vkeyboard_device, event);
usleep(SLEEP_TIME);
            event.value = 0;
            send_key_to_device(vkeyboard_device, event);
usleep(SLEEP_TIME);
            if (key.is_shifted)
                send_shift_up();
        }
        send_sync(vkeyboard_device);
        return;
    }

    char command[256];
    snprintf(command, 256, "wl-copy %s", string);
    int status = system(command);
    if (status < 0)
    {
        perror("system");
        exit(ECOPY);
    }
    struct input_event control_event = {
        .type = EV_KEY,
        .code = KEY_LEFTCTRL,
        .value = 1};
    struct input_event v_event = {
        .type = EV_KEY,
        .code = KEY_V,
        .value = 1};
    send_key_to_device(device_fd, control_event);
    usleep(SLEEP_TIME);
    send_sync(device_fd);
    send_key_to_device(device_fd, v_event);
    usleep(SLEEP_TIME);
    send_sync(device_fd);
    control_event.value = 0;
    send_key_to_device(device_fd, control_event);
    usleep(SLEEP_TIME);
    send_sync(device_fd);
    v_event.value = 0;
    send_key_to_device(device_fd, v_event);
    send_sync(device_fd);
}

void init_trie(struct trie *trie, struct key *key)
{
    trie->character = '\0';
    trie->is_shifted = false;
    trie->parent = NULL;
    trie->is_leaf = false;
    trie->expansion = NULL;
    trie->size = 0;
    if (key != NULL)
    {
        trie->character = key->character;
        trie->keycode = key->keycode;
        trie->is_shifted = key->is_shifted;
    }
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        trie->next[i] = NULL;
    }
}

void push_trie(char *key, char *expansion)
{
    struct trie *current_trie = TRIE;
    for (size_t i = 0; i < strlen(key); i++)
    {
        char character = key[i];
        struct key key = get_key_from_char(character);
        int position = key.position;
        if (current_trie->next[position] == NULL)
        {
            current_trie->next[position] = malloc(sizeof(struct trie));
            init_trie(current_trie->next[position], &key);
            current_trie->next[position]->size = current_trie->size + 1;
            current_trie->next[position]->parent = current_trie;
        }
        current_trie = current_trie->next[position];
    }
    current_trie->is_leaf = true;
    current_trie->expansion = malloc(strlen(expansion) + 1);
    strcpy(current_trie->expansion, expansion);
}

void init_virtual_device(int vkeyboard_device)
{
    struct uinput_setup usetup = {0};

    int status;
    // setup as keyboard
    if ((status = ioctl(vkeyboard_device, UI_SET_EVBIT, EV_KEY)) < 0)
    {
        printf("Error initializing virtual input\n");
        exit(EINIT);
    }
    // setup keys to emit
    if ((status = ioctl(vkeyboard_device, UI_SET_KEYBIT, KEY_BACKSPACE)) < 0)
    {
        printf("Error adding key to virtual input : %d\n", KEY_BACKSPACE);
        exit(EADD);
    }
    if ((status = ioctl(vkeyboard_device, UI_SET_KEYBIT, KEY_LEFTCTRL)) < 0)
    {
        printf("Error adding key to virtual input : %d\n", KEY_LEFTCTRL);
        exit(EADD);
    }
    for (size_t i = 0; i < LINUX_KEYS; i++)
    {
        if ((status = ioctl(vkeyboard_device, UI_SET_KEYBIT, key_codes[i])) < 0)
        {
            printf("Error adding key to virtual input : %ld\n", key_codes[i]);
            exit(EADD);
        }
    }

    memset(&usetup, 0, sizeof(usetup));
    usetup.id.bustype = BUS_USB;
    usetup.id.vendor = 1187;
    usetup.id.product = 1999;
    strcpy(usetup.name, "keydogger");

    if ((status = ioctl(vkeyboard_device, UI_DEV_SETUP, &usetup)) < 0)
    {
        printf("Error setting up virtual device\n");
        exit(ESETUP);
    }
    if ((status = ioctl(vkeyboard_device, UI_DEV_CREATE)) < 0)
    {
        printf("Error creating up virtual device\n");
        exit(ECREATE);
    };
}

void keydogger_daemon()
{
    fkeyboard_device = open(KEYBOARD_DEVICE, O_RDWR | O_APPEND, NULL);
    bool is_shifted = false;

    if (fkeyboard_device < 0)
    {
        printf("Error opening %s\n", KEYBOARD_DEVICE);
        exit(EOPEN);
    }
    vkeyboard_device = open(UINPUT_PATH, O_WRONLY);
    if (vkeyboard_device < 0)
    {
        printf("Error reading from %s\n", UINPUT_PATH);
        exit(EOPEN);
    }

    init_virtual_device(vkeyboard_device);
    struct input_event event = {0};
    struct trie *current_trie = TRIE;
    while (1)
    {
        int read_inputs = read(fkeyboard_device, &event, sizeof(struct input_event));
        if (read_inputs < 0)
        {
            printf("Error reading from %s\n", KEYBOARD_DEVICE);
            exit(EREAD);
        }
        // ignore events other than key
        if (event.type != EV_KEY)
        {
            continue;
        }
        // ignore if keydogger doesnt recognize the key
        if (valid_key_code(event.code) == false)
        {
            continue;
        }

        // Handle shift down
        if (event.value == 1 && (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT))
        {
            is_shifted = true;
            continue;
        }

        // Handle shift up
        if (event.value == 0 && (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT))
        {
            is_shifted = false;
            continue;
        }

        // ignore shift keys
        if (event.code == KEY_LEFTSHIFT || event.code == KEY_RIGHTSHIFT)
        {
            continue;
        }

        // ignore key up event
        if (event.value == 0)
        {
            continue;
        }

        int event_code;
        if (is_shifted)
            event_code = event.code | FLAG_UPPERCASE;
        else
            event_code = event.code;
        int position = get_position_from_event_code((size_t)event_code);

        // if next doesnt match trigger, reset
        if (current_trie->next[position] == NULL)
        {
            current_trie = TRIE;
            continue;
        }

        struct trie *next = current_trie->next[position];
        // if next doesnt match trigger, reset
        if ((event_code ^ char_codes[position]) != 0)
        {
            current_trie = TRIE;
            continue;
        }
        // if next is terminal, expand it
        if (next->is_leaf)
        {
            event.value = 0;
            send_key_to_device(fkeyboard_device, event);
            send_sync(fkeyboard_device);
            send_backspace(vkeyboard_device, next->size);
            send_to_keyboard(vkeyboard_device, next->expansion);
            current_trie = TRIE;
        }
        else
        {
            current_trie = next;
        }
    }
}

void daemonize_keydogger()
{
    int fd;
    pid_t pid;
    pid_t sid;
    sigset_t sigset;
    for (fd = sysconf(_SC_OPEN_MAX); fd >= 0; fd--)
        close(fd);

    for (int i = 1; i < _NSIG; i++)
    {
        if (i != SIGKILL && i != SIGSTOP)
            signal(i, SIG_DFL);
    }

    sigemptyset(&sigset);
    sigprocmask(SIG_SETMASK, &sigset, NULL);

    if ((pid = fork()) < 0)
    {
        printf("Error forking 1\n");
        exit(EFORK);
    }
    // exit Parent
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    if ((sid = setsid()) < 0)
    {
        printf("Error upgrading to session leader\n");
        exit(ELEAD);
    }

    if ((pid = fork()) < 0)
    {
        printf("Error forking 2\n");
        exit(EFORK);
    }
    // exit Child1
    if (pid > 0)
    {
        exit(EXIT_SUCCESS);
    }

    if (prctl(PR_SET_NAME, DAEMON) < 0)
    {
        printf("Error setting name for process\n");
        exit(ERENAM);
    }

    fd = open("/var/log/keydogger.log", O_RDWR | O_CREAT | O_APPEND);
    if (fd < 0)
    {
        printf("Error opening %s\n", "/var/log/keydogger.log");
        exit(EOPEN);
    }
    dup2(fd, STDIN_FILENO);
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);

    umask(0);

    if (chdir("/") < 0)
    {
        printf("Error changing directory to /");
        exit(ECHDIR);
    }

    keydogger_daemon();
}

bool check_priveleges()
{
    if (environ == NULL)
    {
        printf("Error accessing ENV variables\n");
        exit(EENV);
    }
    int i = 0;
    while (environ[i] != NULL)
    {
        if (strncmp("USER=ROOT", environ[i], 9) == 0)
        {
            return true;
        }
        if (strncmp("SUDO_COMMAND", environ[i], 12) == 0)
        {
            return true;
        }
        i++;
    }
    return false;
}

int is_running()
{
    char command[50];
    int items_read = snprintf(command, 50, "pgrep -x %s", DAEMON);
    pid_t pid;
    if (items_read < 0)
    {
        printf("Could not retrive process\n");
        exit(EPGREP);
    }
    if (items_read > 50)
    {
        printf("Command size too long\n");
        exit(ECMD);
    }
    FILE *pgrep;
    if ((pgrep = popen(command, "r")) == NULL)
    {
        printf("Unable to check if \"%s\" daemon is running or not.\n", DAEMON);
        exit(EPIPE);
    }
    items_read = fscanf(pgrep, "%d", &pid);
    pclose(pgrep);
    if (items_read != EOF && pid > 0)
    {
        return pid;
    }
    return -1;
}

void print_trie(struct trie *trie, size_t level)
{
    if (level == 0)
        printf("|");
    if (trie == NULL)
        return;
    for (size_t i = 0; i < level; i++)
    {
        printf("-");
    }
    printf("%c", trie->character);
    if (trie->is_leaf)
    {
        printf(" = %s\n", trie->expansion);
    }
    else
    {
        printf("\n");
    }
    for (size_t i = 0; i < READABLE_KEYS; i++)
    {
        print_trie(trie->next[i], level + 1);
    }
}

int main(int argc, char *argv[])
{
    setlocale(LC_ALL, "");
    set_environment();
    if (argc < 2)
    {
        printf("Usage error: keydogger start | stop | status | debug | viz\n");
        exit(EUSAGE);
    }

    if (!check_priveleges())
    {
        printf("Need sudo priveleges\n");
        exit(EPERM);
    }

    pid_t pid;
    pid = is_running();
    if (strcmp(argv[1], "start") == 0)
    {
        if (pid > 0)
        {
            printf("Already running at pid %d\n", pid);
            exit(EXIT_SUCCESS);
        }
        read_from_rc(NULL);
        daemonize_keydogger();
    }
    else if (strcmp(argv[1], "status") == 0)
    {
        if (pid > 0)
        {
            printf("keydogger running at pid %d\n", pid);
            exit(EXIT_SUCCESS);
        }
        printf("Not running\n");
        exit(EXIT_SUCCESS);
    }

    else if (strcmp(argv[1], "stop") == 0)
    {
        if (pid < 0)
        {
            printf("Not running\n");
            exit(EXIT_SUCCESS);
        }
        cleanup();
        int kill_status = kill(pid, SIGTERM);
        if (kill_status < 0)
        {
            kill(pid, SIGKILL);
        }
        exit(EXIT_SUCCESS);
    }
    else if (strcmp(argv[1], "debug") == 0)
    {
        if (pid > 0)
        {
            kill(pid, SIGTERM);
        }
        read_from_rc(DEBUG_RC_PATH);
        keydogger_daemon();
    }
    else if (strcmp(argv[1], "viz") == 0)
    {
        read_from_rc(DEBUG_RC_PATH);
        print_trie(TRIE, 0);
    }
    else
    {
        printf("Usage error: keydogger start | stop | status | debug | viz\n");
        exit(EUSAGE);
    }
}
