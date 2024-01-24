#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>

MemControl MemController;

void MemController_init(){
    for (size_t i = 0; i < MAX_PARAM_PTR; i++)
    {
        for (size_t j = 0; j < MAX_PTR_DEPTH; j++)
        {
            MemController.fake_addr_buffer[i][j] = &(MemController.fake_addr_buffer[i][j+1]);
        }   
    }
}


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
void* get_fake_addr_list(int idx, int depth){
    if(depth < 0)return NULL;
    return MemController.fake_addr_buffer[idx][MAX_PTR_DEPTH - 1 - depth];
}

void recover_fake_addr_list(int idx, int depth){
    for (size_t i = MAX_PTR_DEPTH - 1 - depth; i < MAX_PTR_DEPTH - 1; i++)
    {
        MemController.fake_addr_buffer[idx][i] = &MemController.fake_addr_buffer[idx][i+1];
    }
    MemController.fake_addr_buffer[idx][MAX_PTR_DEPTH] = NULL;
}
