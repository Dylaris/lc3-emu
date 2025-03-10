#include "lc3.h"

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
    result.buf = buf;
    result.size = size; 

    return result;
}

struct termios oldt;

void disable_input_buf()
{
    tcgetattr(STDIN_FILENO, &oldt);
    struct termios newt = oldt;
    newt.c_lflag &= ~ICANON & ~ECHO;
    tcsetattr(STDIN_FILENO, TCSANOW, &newt);
}

void restore_input_buf()
{
    tcsetattr(STDIN_FILENO, TCSANOW, &oldt);
}

int main(int argc, char **argv)
{
    if (argc != 2) ERR("<USE>: ./lc3-vm program");

    disable_input_buf();
    atexit(restore_input_buf);

    file_content prog = read_file(argv[1]);
    u16 start_pc = *((u16 *) prog.buf);
    lc3 *vm = vm_new(prog.buf + 2, prog.size - 2, start_pc);

    while (vm_exec(vm) != EXEC_END);

    free(vm);
    return 0;
}