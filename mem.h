#ifndef MEM_H
#define MEM_H

#include "hash.h"
#include <stdint.h>
#include <stdlib.h>
#include <wasm.h>
#include <wasmtime.h>

typedef enum {
  TRUNC, // host地址空间的截断地址
  WASM   // wasm地址空间的地址
} MEM_TYPE;

#define MAX_PTR_DEPTH 8
#define WASM_MAX_ADDR_BITS 28 // 如果28位以上的地址为空，则判断我wasm地址

typedef struct {
  uintptr_t offset; // real TRUNC addr = offset << 32 + wasm addr
  uintptr_t base;

  HashMap clAddrRec; // 用于判断cl结构体截断地址
} MemControl;

extern MemControl MemController;

MEM_TYPE get_mem_type(uint32_t addr);

void *get_host_addr(uint32_t wasm_addr, MEM_TYPE tag);
void *get_host_addr_auto(uint32_t wasm_addr);
size_t *get_host_size_t_addr(uint32_t wasm_addr, size_t *size_t_buffer);
void *get_addr64_arg_w(void *buffer[], void *start, size_t length,
                       size_t depth);
void *get_addr64_arg_r(void *buffer[], uint32_t target[], MEM_TYPE mt,
                       size_t length, size_t depth);

void cp_host_addr_to_wasm(uint32_t wasm_addr_buffer[],
                          uintptr_t host_addr_buffer[], size_t count);
void cp_wasm_size_t_to_host(uint32_t *wasm_addr, size_t *host_size_t_buffer,
                            size_t count);

#endif // MEM_H
