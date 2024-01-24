#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasm.h>

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
    case HOST:
        return (void *)(MemController.offset + wasm_addr_64 );
        break;
    case WASM:
        if(wasm_addr == 0){return NULL;}
        else {return (void *)(MemController.base + wasm_addr_64 );}
    default:
        assert(false);
        break;
    }
    
}