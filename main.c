#include "lc3.h"

typedef struct file_content {
    char *buf;
    size_t size;
} file_content;

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

    file_content result = {};
    result.buf = buf;
    result.size = size;

    return result;
}

int main(int argc, char **argv)
{
    if (argc != 2) ERR("<USE>: ./lc3-vm program");

    file_content prog = read_file(argv[1]);
    lc3 *vm = vm_new(prog.buf, prog.size);

    while (vm_exec(vm) != EXEC_END);

    free(vm);
    return 0;
}