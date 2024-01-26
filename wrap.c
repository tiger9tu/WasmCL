#include "wrap.h"
#include "mem.h"
#include <assert.h>
#include <stdio.h>
#include <CL/cl.h>

int register_func_to_linker(
    define_func funcs[], 
    int count, 
    wasmtime_linker_t *linker, 
    const char *module, 
    size_t module_len)
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
    puts("calling clGetPlatformIDS");
    print_args(args,nargs);

    // cl_uint num_entries = args[0].of.i32;
    // uint32_t* platforms = get_host_addr(args[1].of.i32, WASM);

    // cl_platform_id fake_platforms[MAX_PTR_DEPTH];
    // void* addr64_arg_platforms = fake_platforms;
    // if(platforms == NULL){
    //     addr64_arg_platforms = NULL;
    // }else{
    //     make_fake_addr_list(fake_platforms,num_entries);
    // }
    // wasm32位地址不能作为opencl函数参数，因此需要创建64位地址作为参数，调用完后再将结果拷贝到wasm32位地址中
    intptr_t buffer[MAX_PTR_DEPTH];
    intptr_t* addr64_arg_platforms = get_addr64_arg_w(
                                buffer, get_host_addr(args[1].of.i32, WASM), args[0].of.i32, 1);
    
    results[0].of.i32 = clGetPlatformIDs(
        args[0].of.i32, 
        addr64_arg_platforms, 
        get_host_addr(args[2].of.i32, WASM));

    cp_host_addr_to_wasm(get_host_addr(args[1].of.i32, WASM), addr64_arg_platforms, args[0].of.i32);
    
    puts("clGetPlatformIDs finished\n");
    return NULL;
}

wasm_trap_t *clGetDeviceIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    puts("\ncalling getDeviceIDS");
    print_args(args,nargs);
    
    cl_uint num_entries = args[2].of.i32;
    uint32_t* devices = (cl_device_id *)get_host_addr(args[3].of.i32,WASM);

    intptr_t buffer[MAX_PTR_DEPTH];
    intptr_t* addr64_arg_devices = get_addr64_arg_w(buffer, devices,  num_entries, 1);

    results[0].of.i32 = clGetDeviceIDs(
        get_host_addr(args[0].of.i32,TRUNC), 
        args[1].of.i64, 
        args[2].of.i32, 
        addr64_arg_devices, 
        get_host_addr(args[4].of.i32,WASM));

    cp_host_addr_to_wasm(devices, addr64_arg_devices, num_entries);
    
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
    uintptr_t buffer[MAX_PTR_DEPTH];
    uintptr_t* addr64_arg_devices = get_addr64_arg_r(buffer, devices, TRUNC, num_devices,1);

    results[0].of.i32 = clCreateContext(
        get_host_addr(args[0].of.i32, WASM),
        num_devices,
        addr64_arg_devices,
        get_host_addr(args[3].of.i32, WASM),
        get_host_addr(args[4].of.i32, WASM),
        get_host_addr(args[5].of.i32, WASM));
    
    puts("clCreateContext finished\n");
    return NULL;
}


wasm_trap_t *clCreateProgramWithSource_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){
    check_offset();
    puts("\ncalling clCreateProgramWithSource");
    print_args(args,nargs);

    // cl_uint count = args[1].of.i32;
    // char** strings = get_host_addr(args[2].of.i32, WASM);
    // char* fake_strings[MAX_PTR_DEPTH];
    
    // char** arg_fake_strings;
    // if(strings == NULL){
    //     arg_fake_strings = NULL;
    // }else{
    //     for (size_t i = 0; i < count; i++)
    //     {
    //         fake_strings[i] = get_host_addr(strings[i],WASM);
    //     }
    //     arg_fake_strings = fake_strings;
    // }
    uintptr_t buffer[MAX_PTR_DEPTH];
    uintptr_t* addr64_arg_strings = get_addr64_arg_r(
        buffer, get_host_addr(args[2].of.i32, WASM), WASM,args[1].of.i32, 1);

    results[0].of.i32 = clCreateProgramWithSource(
        get_host_addr(args[0].of.i32, TRUNC),
        args[1].of.i32,
        addr64_arg_strings,
        get_host_addr(args[3].of.i32, WASM),
        get_host_addr(args[4].of.i32, WASM));
    
    puts("clCreateProgramWithSource finished\n");
    return NULL;
}

wasm_trap_t *clBuildProgram_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){
    
    print_args(args, nargs);
    intptr_t buffer[MAX_PTR_DEPTH];
    intptr_t* addr64_arg_device_list = get_addr64_arg_r(
        buffer, get_host_addr(args[2].of.i32,WASM),TRUNC,args[1].of.i32,1);

    results[0].of.i32 = clBuildProgram(
        get_host_addr(args[0].of.i32, TRUNC),
        args[1].of.i32,
        addr64_arg_device_list,
        get_host_addr(args[3].of.i32, WASM),
        get_host_addr(args[4].of.i32, WASM),
        get_host_addr(args[5].of.i32, WASM)
    );

    return NULL;
}


wasm_trap_t *clGetProgramBuildInfo_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    results[0].of.i32 = clGetProgramBuildInfo(
        get_host_addr(args[0].of.i32, WASM),
        get_host_addr(args[1].of.i32, TRUNC),
        args[2].of.i32,
        args[3].of.i32,
        get_host_addr(args[4].of.i32, WASM),
        get_host_addr(args[5].of.i32, WASM)
    );    
    
    return NULL;
}


wasm_trap_t *clCreateBuffer_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    puts("calling clCreateBuffer");
    print_args(args,nargs);
    results[0].of.i32 = clCreateBuffer(
        get_host_addr(args[0].of.i32, TRUNC),
        args[1].of.i64,
        args[2].of.i32,
        get_host_addr(args[3].of.i32, WASM),
        get_host_addr(args[4].of.i32, WASM)
    );
    puts("clCreateBuffer finished");
    return NULL;
}

wasm_trap_t *clCreateCommandQueue_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    puts("calling clCreateCommandQueue");
    print_args(args,nargs);
    results[0].of.i32 = clCreateCommandQueue(
        get_host_addr(args[0].of.i32, TRUNC),
        get_host_addr(args[1].of.i32, TRUNC),
        args[2].of.i64,
        get_host_addr(args[3].of.i32, WASM)
    );

    puts("clCreateCommandQueue finished");
    return NULL;
}


wasm_trap_t *clCreateKernel_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    puts("calling clCreateKernel");
    print_args(args,nargs);
    results[0].of.i32 = clCreateKernel(
        get_host_addr(args[0].of.i32, TRUNC),
        get_host_addr(args[1].of.i32, WASM),
        get_host_addr(args[2].of.i32, WASM)
    );
    puts("clCreateKernel finished");
    return NULL;
}



wasm_trap_t *clSetKernelArg_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    puts("calling clSetKernelArg");
    print_args(args,nargs);

    intptr_t buffer[MAX_PTR_DEPTH];
    intptr_t* addr64_arg_value = get_addr64_arg_r(
        buffer,get_host_addr(args[3].of.i32, WASM),TRUNC,1,1);

    size_t arg_size = addr64_arg_value ? args[2].of.i32 * 2: args[2].of.i32;

    results[0].of.i32 = clSetKernelArg(
        get_host_addr(args[0].of.i32, TRUNC),
        args[1].of.i32,
        arg_size,
        addr64_arg_value
    );
    
    puts("clSetKernelArg finished");
    return NULL;
}



wasm_trap_t *clEnqueueNDRangeKernel_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    puts("calling clEnqueueNDRangeKernel");
    print_args(args,nargs);

    intptr_t buffer_1[MAX_PTR_DEPTH];
    intptr_t addr64_arg_event_wait_list = 
        get_addr64_arg_r(buffer_1,get_host_addr_auto(args[7].of.i32),TRUNC,args[6].of.i32, 1);

    intptr_t buffer_2[MAX_PTR_DEPTH];
    intptr_t addr64_arg_event = 
    get_addr64_arg_w(buffer_2, get_host_addr_auto(args[8].of.i32), 1, 1);
    
    results[0].of.i32 = clEnqueueNDRangeKernel(
        get_host_addr_auto(args[0].of.i32),
        get_host_addr_auto(args[1].of.i32),
        args[2].of.i32,
        get_host_addr_auto(args[3].of.i32),
        get_host_addr_auto(args[4].of.i32),
        get_host_addr_auto(args[5].of.i32),
        args[6].of.i32,
        addr64_arg_event_wait_list,
        addr64_arg_event
    );
    cp_host_addr_to_wasm(get_host_addr_auto(args[8].of.i32), addr64_arg_event, 1);
    
    puts("clEnqueueNDRangeKernel finished");
    return NULL;
}


wasm_trap_t *clEnqueueReadBuffer_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults){

    puts("calling clEnqueueReadBuffer");
    print_args(args,nargs);

    intptr_t buffer_1[MAX_PTR_DEPTH];
    intptr_t addr64_arg_event_wait_list = 
        get_addr64_arg_r(buffer_1,get_host_addr_auto(args[7].of.i32),TRUNC,args[6].of.i32, 1);

    intptr_t buffer_2[MAX_PTR_DEPTH];
    intptr_t addr64_arg_event = 
    get_addr64_arg_w(buffer_2, get_host_addr_auto(args[8].of.i32), 1, 1);

    printf("para 0 = %p;  para 1 = %p\n", get_host_addr_auto(args[0].of.i32),
        get_host_addr_auto(args[1].of.i32) );

    float a[2];

    cl_int ret = clEnqueueReadBuffer(
        get_host_addr_auto(args[0].of.i32),
        get_host_addr_auto(args[1].of.i32),
        1,
        0,
        8,
        a,
        0,
        NULL,NULL
    );

    printf("ret = %d\n",ret);

    // results[0].of.i32 = clEnqueueReadBuffer(
    //     get_host_addr_auto(args[0].of.i32),
    //     get_host_addr_auto(args[1].of.i32),
    //     args[2].of.i32,
    //     args[3].of.i32,
    //     args[4].of.i32,
    //     get_host_addr_auto(args[5].of.i32),
    //     args[6].of.i32,
    //     NULL,NULL
    //     // addr64_arg_event_wait_list,
    //     // addr64_arg_event
    // );
    // printf("result = %d\n",results[0].of.i32);
    // cp_host_addr_to_wasm(get_host_addr_auto(args[8].of.i32), addr64_arg_event, 1);
    
    puts("clEnqueueReadBuffer finished");
    return NULL;
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
    .cb = clCreateProgramWithSource_callback
    },
    {
    .name = "clBuildProgram",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 6,
    .result_type = WASM_I32,
    .cb = clBuildProgram_callback
    },
    {
    .name = "clGetProgramBuildInfo",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32 ,WASM_I32},
    .param_len = 6,
    .result_type = WASM_I32,
    .cb = clGetProgramBuildInfo_callback
    },
    {
    .name = "clCreateBuffer",
    .param_types = {WASM_I32, WASM_I64, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 5,
    .result_type = WASM_I32,
    .cb = clCreateBuffer_callback
    },
    {
    .name = "clCreateCommandQueue",
    .param_types = {WASM_I32, WASM_I32, WASM_I64, WASM_I32},
    .param_len = 4,
    .result_type = WASM_I32,
    .cb = clCreateCommandQueue_callback
    },
    {
    .name = "clCreateKernel",
    .param_types = {WASM_I32, WASM_I32, WASM_I32},
    .param_len = 3,
    .result_type = WASM_I32,
    .cb = clCreateKernel_callback
    },
    {
    .name = "clSetKernelArg",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 4,
    .result_type = WASM_I32,
    .cb = clSetKernelArg_callback
    },
    {
    .name = "clEnqueueNDRangeKernel",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32,WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 9,
    .result_type = WASM_I32,
    .cb = clEnqueueNDRangeKernel_callback
    },
    {
    .name = "clEnqueueReadBuffer",
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32,WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},
    .param_len = 9,
    .result_type = WASM_I32,
    .cb = clEnqueueReadBuffer_callback
    }
};



