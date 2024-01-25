#ifndef MEM_H
#define MEM_H

#include <stdint.h>

typedef enum {
    TRUNC,  // host地址空间的截断地址
    WASM    // wasm地址空间的地址
} MEM_TYPE;

#define MAX_PTR_DEPTH 8
typedef struct {
    uintptr_t offset; // real TRUNC addr = offset << 32 + wasm addr
    uintptr_t base;
} MemControl;

extern MemControl MemController;

// void MemController_init();

void check_offset();
void *get_host_addr(uint32_t wasm_addr, MEM_TYPE tag);
void make_fake_addr_list(intptr_t* start, size_t count);
void* get_addr64_arg_w(void* buffer[], void* start, size_t length, size_t depth);
void* get_addr64_arg_r(void* buffer[], void* target[], MEM_TYPE mt, size_t length, size_t depth);
void cp_host_addr_to_wasm(uint32_t wasm_addr_buffer[], uintptr_t host_addr_buffer[], size_t count);
#endif // MEM_H
