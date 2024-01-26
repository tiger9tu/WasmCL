#ifndef WRAP_H
#define WRAP_H

#include <wasm.h>
#include <wasmtime.h>

#define MAX_CL_NAME 30
#define MAX_CL_PARAM 12

typedef struct
{
    char name[MAX_CL_NAME];
    wasm_valkind_t param_types[MAX_CL_PARAM];
    size_t param_len;
    wasm_valkind_t result_type;
    wasmtime_func_async_callback_t cb;

} define_func;

#define FUNC_NUM 12

int register_func_to_linker(define_func funcs[], 
    int count, wasmtime_linker_t *linker, 
    const char *module, 
    size_t module_len);

extern define_func func_array[];

#endif // WRAP_H
