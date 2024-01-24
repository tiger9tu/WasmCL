#include "wrap.h"
#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <CL/cl.h>

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
    },
    {
    .name = "clCreateProgramWithSource",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 5,
    .result_type = WASM_I32,
    .cb = clGetPlatformIDs_callback
    },
    {
    .name = "clBuildProgram",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 6,
    .result_type = WASM_I32,
    .cb = clGetDeviceIDs_callback
    },
    {
    .name = "clGetProgramBuildInfo",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32 ,WASM_I32},
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

    cl_uint num_entries = args[0].of.i32;
    uint32_t* platforms = get_host_addr(args[1].of.i32, WASM);

    cl_platform_id* fake_platforms;
    if(platforms == NULL){
        fake_platforms = NULL;
    }else{
        fake_platforms = get_fake_addr_list(0,num_entries);
    }
     
    results[0].of.i32 = clGetPlatformIDs(
        num_entries, 
        fake_platforms, 
        get_host_addr(args[2].of.i32, WASM));

    if(platforms)
        for (size_t i = 0; i < num_entries; i++)
        {
            platforms[i] = (uint32_t)fake_platforms[i];
        }
    
    recover_fake_addr_list(0,num_entries);
    puts("clGetPlatformIDs finished\n");
    return NULL;
}

wasm_trap_t *clGetDeviceIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    check_offset();
    puts("\ncalling getDeviceIDS");
    print_args(args,nargs);
    
    cl_uint num_entries = args[2].of.i32;
    uint32_t* devices = (cl_device_id *)get_host_addr(args[3].of.i32,WASM);

    cl_device_id *fake_devices;
    if(devices == NULL){
        fake_devices = NULL;
    }else{
        fake_devices = get_fake_addr_list(0, num_entries);
    }

    results[0].of.i32 = clGetDeviceIDs(
        get_host_addr(args[0].of.i32,TRUNC), 
        args[1].of.i64, 
        args[2].of.i32, 
        fake_devices, 
        get_host_addr(args[4].of.i32,WASM));

    for (size_t i = 0; i < num_entries; i++)
    {
        printf("fake devices %d = %p\n", i, fake_devices[i]);
        devices[i] = (uint32_t)fake_devices[i];
    }
    printf("in clGetDeviceIDs, devices = %p, devices[0] = %p\n", devices, devices[0]);
    
    recover_fake_addr_list(0, num_entries);
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

    cl_uint num_devices =  args[1].of.i32;
    uint32_t* devices = get_host_addr(args[2].of.i32, WASM);
    printf("in clCreateContext, devices = %p\n", devices);
    cl_device_id *fake_devices;
    if(devices == NULL){
        fake_devices = NULL;
    }else{
        fake_devices = get_fake_addr_list(0, num_devices);
        for (size_t i = 0; i < num_devices; i++)
        {
            printf("hil\n");
            printf("host addr devices %d = %p, devices[0] = %p , hostaddr_devices[0] = %p\n",i ,devices, devices[0], get_host_addr(devices[i], TRUNC));
            printf("base = %p\n", MemController.base);
            printf("offset = %p\n", MemController.offset);
            fake_devices[i] = get_host_addr(devices[i], TRUNC);
        }
    }

    results[0].of.i32 = clCreateContext(
        get_host_addr(args[0].of.i32, WASM),
        num_devices,
        fake_devices,
        get_host_addr(args[3].of.i32, WASM),
        get_host_addr(args[4].of.i32, WASM),
        get_host_addr(args[5].of.i32, WASM));
    
    recover_fake_addr_list(0,num_devices);
    puts("clCreateContext finished\n");
    return NULL;
}


// wasm_trap_t *clCreateProgramWithSource_callback(
//     void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
//     size_t nargs, wasmtime_val_t *results, size_t nresults){
//     check_offset();
//     puts("\ncalling clCreateProgramWithSource");
//     print_args(args,nargs);
//     // results[0].of.i32 = clCreateProgramWithSource(
//     // )
//     );
    
// }

// wasm_trap_t *clBuildProgram_callback(
//     void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
//     size_t nargs, wasmtime_val_t *results, size_t nresults);

// wasm_trap_t *clGetProgramBuildInfo_callback(
//     void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
//     size_t nargs, wasmtime_val_t *results, size_t nresults);



