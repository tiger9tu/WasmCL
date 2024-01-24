#ifndef MEM_H
#define MEM_H

#include <stdint.h>

typedef enum {
    TRUNC,  // host地址空间的截断地址
    WASM    // wasm地址空间的地址
} MEM_TYPE;

#define MAX_PARAM_PTR 8
#define MAX_PTR_DEPTH 8
typedef struct {
    uintptr_t offset; // real TRUNC addr = offset << 32 + wasm addr
    uintptr_t base;
    uintptr_t fake_addr_buffer[MAX_PARAM_PTR][MAX_PTR_DEPTH];
} MemControl;

extern MemControl MemController;

void MemController_init();

void check_offset();
void *get_host_addr(uint32_t wasm_addr, MEM_TYPE tag);
void* get_fake_addr_list(int idx, int depth);
void recover_fake_addr_list(int idx, int depth);
#endif // MEM_H
