#include "lc3.h"

typedef struct file_content {
    char *buf;
    size_t size;
} file_content;

static file_content read_file(const char *filename)
{
    FILE *file = fopen(filename, "rb");
    if (!file) ERR("open file");

    fseek(file, SEEK_END, 0);
    size_t size = (size_t) ftell(file);
    fseek(file, SEEK_SET, 0);

    file_content result = {};
    result.size = size;
    size_t read = fread(result.buf, 1, size, file);
    if (read != size) ERR("read file");

    return result;
}

int main(int argc, char **argv)
{
    if (argc != 2) ERR("<USE>: ./lc3-vm program");

    file_content prog = read_file(argv[1]);
    lc3 *vm = new_vm(prog.buf, prog.size);

    free(vm);
    exit(0);
}