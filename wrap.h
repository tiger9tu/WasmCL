#ifndef WRAP_H
#define WRAP_H

#include <wasm.h>
#include <wasmtime.h>

#define MAX_CL_NAME 30
#define MAX_CL_PARAM 8

typedef struct
{
    char name[MAX_CL_NAME];
    wasm_valkind_t param_types[MAX_CL_PARAM];
    size_t param_len;
    wasm_valkind_t result_type;
    wasmtime_func_async_callback_t cb;

} define_func;

#define FUNC_NUM 3

wasm_trap_t *clGetPlatformIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults);

wasm_trap_t *clGetDeviceIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults);

wasm_trap_t *clCreateContext_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults);

wasm_trap_t *clCreateProgramWithSource_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults);

wasm_trap_t *clBuildProgram_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults);

wasm_trap_t *clGetProgramBuildInfo_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults);


int register_func_to_linker(define_func funcs[], 
int count, wasmtime_linker_t *linker, 
const char *module, 
size_t module_len);


extern define_func func_array[];

#endif // WRAP_H
