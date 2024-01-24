#include "wrap.h"
#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <CL/cl.h>


define_func func_array[30] = {
    {
    .name = "clGetPlatformIDs",
    .param_types = {
        WASM_I32, WASM_I32, WASM_I32},
    .param_len = 3,
    .result_type = WASM_I32,
    .cb = clGetPlatformIDs_callback
    },
    {
    .name = "clGetDeviceIDs",
    .param_types = {WASM_I32, WASM_I64, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 5,
    .result_type = WASM_I32,
    .cb = clGetDeviceIDs_callback
    },
    {
    .name = "clCreateContext",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32,WASM_I32 ,WASM_I32},
    .param_len = 6,
    .result_type = WASM_I32,
    .cb = clCreateContext_callback
    }
};

void print_args(const wasmtime_val_t *args,
    size_t nargs){
        for (size_t i = 0; i < nargs; i++)
        {
            printf("arg[%d] = %p\n", i,args[i].of.i32);
        }
}

wasm_trap_t *clGetPlatformIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    check_offset();
    puts("calling clGetPlatformIDS");
    print_args(args,nargs);

    results[0].of.i32 = clGetPlatformIDs(
    args[0].of.i32, 
    get_host_addr(args[1].of.i32, WASM), 
    get_host_addr(args[2].of.i32, WASM));
    
    return NULL;
}

wasm_trap_t *clGetDeviceIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    check_offset();
    puts("\ncalling getDeviceIDS");
    print_args(args,nargs);
    
    results[0].of.i32 = clGetDeviceIDs(
    (cl_platform_id)get_host_addr(args[0].of.i32,HOST), 
    args[1].of.i64, 
    args[2].of.i32, 
    (cl_device_id *)get_host_addr(args[3].of.i32,WASM), 
    (cl_uint *)get_host_addr(args[4].of.i32,WASM));

    printf("clDeviceID = %p\n", *(cl_device_id *)get_host_addr(args[3].of.i32,WASM));
    puts("finished calling getDeviceIDS");
    return NULL;
}


wasm_trap_t *clCreateContext_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    check_offset();
    puts("\ncalling clCreateContext");
    print_args(args,nargs);
    *(cl_device_id *)get_host_addr(args[2].of.i32, WASM) = get_host_addr(*(cl_device_id *)get_host_addr(args[2].of.i32, WASM), HOST);
    printf("device = %p, errocode_ret = %p\n", (const cl_device_id *)get_host_addr(args[2].of.i32, WASM),
    (cl_int *)get_host_addr(args[5].of.i32, WASM));

    results[0].of.i32 = clCreateContext(
    (const cl_context_properties *)get_host_addr(args[0].of.i32, WASM),
    (cl_uint)args[1].of.i32,
    (const cl_device_id *)get_host_addr(args[2].of.i32, WASM),
    get_host_addr(args[3].of.i32, WASM),
    (void * )get_host_addr(args[4].of.i32, WASM),
    (cl_int *)get_host_addr(args[5].of.i32, WASM));

    puts("finished calling clCreateContext");
    return NULL;
}


int register_func_to_linker(define_func funcs[], int count, wasmtime_linker_t *linker, const char *module, size_t module_len)
{
    for (size_t i = 0; i < count; i++)
    {
        define_func tmp = funcs[i];

        wasm_valtype_t *params[MAX_CL_PARAM];
        for (size_t j = 0; j < tmp.param_len; j++)
        {
            params[j] = wasm_valtype_new(tmp.param_types[j]);
        }

        wasm_valtype_t *result[1] = {wasm_valtype_new(tmp.result_type)};

        wasm_valtype_vec_t params_vec, result_vec;
        wasm_valtype_vec_new(&params_vec, tmp.param_len, params);
        wasm_valtype_vec_new(&result_vec, 1 /*in c there is only 1 result*/, result);

        wasm_functype_t *tmp_func_type = wasm_functype_new(&params_vec, &result_vec);

        int i = 42;

        wasmtime_error_t *error = wasmtime_linker_define_func(
            linker,
            "env", 
            3,
            tmp.name,
            strlen(tmp.name),
            tmp_func_type,
            tmp.cb,
            &i,
            NULL);

        wasm_functype_delete(tmp_func_type);
    }
}
