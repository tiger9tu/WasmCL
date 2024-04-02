#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>

MemControl MemController;

// 获取堆空间的高32位地址
void check_offset() { // heap offset =  current heap address >> 32 << 32
  int var;
  int *heap_ptr = (int *)malloc(sizeof(var));
  free(heap_ptr);
  uintptr_t cur_offset = ((uint64_t)heap_ptr >> 32) << 32;

  if (MemController.offset == 0) {
    MemController.offset = cur_offset;
  } else {
    if (MemController.offset != cur_offset) {
      printf("warning: offset changed from %llx to %llx", MemController.offset,
             cur_offset);
      MemController.offset = cur_offset;
    }
  }
}

MEM_TYPE get_mem_type(uint32_t addr) {
  return get(&MemController.clAddrRec, addr) ? TRUNC : WASM;
}

void *get_host_addr(uint32_t wasm_addr, MEM_TYPE tag) {
  if (wasm_addr >> WASM_MAX_ADDR_BITS != 0 && tag == WASM) {
    printf("warning!, tag may not be WASM as wasm_addr = %p\n", wasm_addr);
  }

  uintptr_t wasm_addr_64 = (uintptr_t)wasm_addr;
  switch (tag) {
  case TRUNC:
    return (void *)(MemController.offset + wasm_addr_64);
  case WASM:
    if (wasm_addr == 0) {
      return NULL;
    } else {
      return (void *)(MemController.base + wasm_addr_64);
    }
  default:
    return NULL;
  }
}

void *get_host_addr_auto(uint32_t wasm_addr) {
  if (wasm_addr == NULL)
    return NULL;
  return get_host_addr(wasm_addr, get_mem_type(wasm_addr));
}

size_t *get_host_size_t_addr(uint32_t wasm_addr, size_t *size_t_buffer) {
  unsigned int *hostPtr = get_host_addr_auto(wasm_addr);
  if (hostPtr == NULL) {
    return NULL;
  } else {
    *size_t_buffer = (size_t)*hostPtr;
    printf("size_t value = %zu\n\n", *size_t_buffer);
    return size_t_buffer;
  }
}

// 获得一个连续的链表
void make_fake_addr_list(intptr_t *start, size_t count) {
  if (count < 0)
    return NULL;
  for (size_t i = 0; i < count - 1; i++) {
    start[i] = &start[i + 1];
  }
  start[count - 1] = NULL;
}

// 创建地址参数，提供给opencl函数, 该地址参数会被写
void *get_addr64_arg_w(void *buffer[], void *start, size_t length,
                       size_t depth) {
  check_offset();
  if (start == NULL)
    return NULL;

  make_fake_addr_list(buffer, depth + length - 1);
  buffer[depth - 1] = NULL;
  return buffer;
}

// 创建地址参数，提供给opencl函数, 该地址参数只读
void *get_addr64_arg_r(void *buffer[], uint32_t target[], MEM_TYPE mt,
                       size_t length, size_t depth) {
  check_offset();
  if (target == NULL)
    return NULL;

  make_fake_addr_list(buffer, depth + length - 1);
  for (size_t i = 0; i < length; i++) {
    buffer[depth - 1 + i] = get_host_addr(target[i], mt);
  }
  return buffer;
}

void cp_host_addr_to_wasm(uint32_t wasm_addr_buffer[],
                          uintptr_t host_addr_buffer[], size_t count) {
  if (host_addr_buffer == NULL || wasm_addr_buffer == NULL)
    return;
  for (size_t i = 0; i < count; i++) {
    uint32_t trunc_addr = (uint32_t)host_addr_buffer[i];
    wasm_addr_buffer[i] = trunc_addr;
    insert(&MemController.clAddrRec, trunc_addr, host_addr_buffer[i]);
  }
}

void cp_wasm_size_t_to_host(uint32_t *wasm_addr, size_t *host_size_t_buffer,
                            size_t count) {
  for (size_t i = 0; i < count; i++) {
    host_size_t_buffer[i] = (size_t)wasm_addr[i];
  }
}