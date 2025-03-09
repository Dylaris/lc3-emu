#include "lc3.h"
#include <termios.h>
#include <unistd.h>

typedef struct file_content {
    char *buf;
    size_t size;
} file_content;

/* swap (big endian <-> little endian) */
void swap_16(u16 *val) { *val = (*val >> 8) | (*val << 8); }

static file_content read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file) ERR("open file");

    fseek(file, 0, SEEK_END);
    size_t size = (size_t) ftell(file);
    fseek(file, 0, SEEK_SET);

    char *buf = malloc(size);
    if (!buf) ERR("buf malloc");

    size_t read = fread(buf, 1, size, file);
    if (read != size) ERR("read file");

    /* convert big endian to little endian */
    for (size_t i = 0; i < size / 2; i++) {
        u16 *ptr = (u16 *) (buf + i * 2);
        swap_16(ptr);
    }

    /* no need the first two bytes */
    file_content result = {};
    result.buf = buf + 2;
    result.size = size - 2;

    return result;
}

struct termios oldt, newt;

void disable_echo()
{
    tcgetattr(STDIN_FILENO, &oldt);
    newt = oldt;
    newt.c_lflag &= ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void enable_echo()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main(int argc, char **argv)
{
    if (argc != 2) ERR("<USE>: ./lc3-vm program");

    disable_echo();
    atexit(enable_echo);
    setvbuf(stdin, NULL, _IONBF, 0);
    setvbuf(stdout, NULL, _IONBF, 0);

    file_content prog = read_file(argv[1]);
    lc3 *vm = vm_new(prog.buf, prog.size);

    while (vm_exec(vm) != EXEC_END);

    free(vm);
    return 0;
}