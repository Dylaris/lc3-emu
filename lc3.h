#ifndef LC3_H
#define LC3_H

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef uint8_t  u8;
typedef uint16_t u16;

/*
 * Memory map
 */
#define TVT_OFF 0x0000 // trap vector table
#define IVT_OFF 0X0100 // interrupt vector table
#define SS_OFF  0x0200 // supervisor stack
#define UM_OFF  0x3000 // user memory
#define DRM_OFF 0xFE00 // device register memory

#define RAM_SIZE 0xFFFF
#define TVT_SIZE (IVT_OFF  - TVT_OFF)
#define IVT_SIZE (SS_OFF   - IVT_OFF)
#define SS_SIZE  (UM_OFF   - SS_OFF )
#define UM_SIZE  (DRM_OFF  - UM_OFF )
#define DRM_SIZE (RAM_SIZE - DRM_OFF)

/*
 * I/O register
 */
#define KBSR_OFF (DRM_OFF + 0) // keyboard status register
#define KBDR_OFF (DRM_OFF + 2) // keyboard data register
#define DSR_OFF  (DRM_OFF + 4) // display status register
#define DDR_OFF  (DRM_OFF + 6) // display data register
#define MCR_OFF  0xFFFE        // machine control register

#define PROG_LOAD_ADDR UM_OFF

#define EXEC_OK   0
#define EXEC_END  1

typedef struct lc3 {
    u16 ram[RAM_SIZE];
    u16 r[8], pc, psr;
    u16 *sp;

    u16 _size; // program size 

    // I/O register
    u16 *kbsr, *kbdr, *dsr, *ddr, *mcr;
} lc3;

lc3 *vm_new(const char *buf, size_t size);
int vm_exec(lc3 *vm);

#define ERR(msg) do { \
        fprintf(stderr, "error: %s at %d in %s\n", msg, __LINE__, __FILE__); \
        exit(1); \
    } while (0)

#endif /* LC3_H */
