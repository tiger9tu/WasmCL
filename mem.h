#ifndef MEM_H
#define MEM_H

#include <stdint.h>

typedef enum {
    HOST,
    WASM
} MEM_TYPE;

typedef struct {
    uintptr_t offset; // real HOST addr = offset << 32 + wasm addr
    uintptr_t base;
} MemControl;

extern MemControl MemController;

void check_offset();
void *get_host_addr(uint32_t wasm_addr, MEM_TYPE tag);

#endif // MEM_H
