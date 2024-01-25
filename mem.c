#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>

MemControl MemController;

// 获取堆空间的高32位地址
void check_offset(){ // heap offset =  current heap address >> 32 << 32
    int var;
    int *heap_ptr = (int*)malloc(sizeof(var));
    free(heap_ptr);
    uintptr_t cur_offset = ((uint64_t)heap_ptr >> 32) << 32;

    if(MemController.offset == 0){
        MemController.offset = cur_offset;
    }
    else
    {
        if(MemController.offset != cur_offset){ 
            printf("warning: offset changed from %llx to %llx", MemController.offset, cur_offset);
            MemController.offset = cur_offset;
        }
    }
}


void* get_host_addr(uint32_t wasm_addr, MEM_TYPE tag){
    uintptr_t wasm_addr_64 = (uintptr_t)wasm_addr;
    switch (tag)  
    {
    case TRUNC: 
        return (void *)(MemController.offset + wasm_addr_64);
    case WASM:
        if(wasm_addr == 0){return NULL;}
        else {
            return (void *)(MemController.base + wasm_addr_64);
            }
    default:
        return NULL;
    }
    
}

// 获得一个连续的链表
void make_fake_addr_list(intptr_t* start, size_t count){
    if(count < 0)return NULL;
    for (size_t i = 0; i < count - 1; i++)
    {
        start[i] = &start[i+1];
    }
    start[count - 1] = NULL;
}

// 创建地址参数，提供给opencl函数, 该地址参数会被写
void* get_addr64_arg_w(void* buffer[], void* start, size_t length, size_t depth){
    check_offset();
    if(start == NULL)return NULL;

    make_fake_addr_list(buffer, depth + length - 1);
    buffer[depth - 1] = NULL;
    return buffer;
}

// 创建地址参数，提供给opencl函数, 该地址参数只读
void* get_addr64_arg_r(void* buffer[], uint32_t target[], MEM_TYPE mt, size_t length, size_t depth){
    check_offset();
    if(target == NULL)return NULL;

    make_fake_addr_list(buffer, depth + length - 1);
    for (size_t i = 0; i < length; i++)
    {
        buffer[depth - 1 + i] = get_host_addr(target[i], mt);   
    }
    return buffer;
}

void cp_host_addr_to_wasm(uint32_t wasm_addr_buffer[], uintptr_t host_addr_buffer[], size_t count){
    if(host_addr_buffer == NULL) return;
    for (size_t i = 0; i < count; i++)
    {
        wasm_addr_buffer[i] = (uint32_t)host_addr_buffer[i];
    }   
}