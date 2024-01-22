#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasi.h>
#include <wasm.h>
#include <wasmtime.h>
#include <CL/cl.h>

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap);

typedef enum{
        HOST,
        WASM
    }MEM_TYPE;

// we must know which is in host memory space and which is in wasm memory space.
// HOST: cl_platform_id cl_device_id 


#define MAX_ADDR_NUM 100000
typedef struct {
    uintptr_t offset; // real HOST addr = offset << 32 + wasm addr
    uintptr_t base;
}MemControl;

MemControl MemController;


void print_args(const wasmtime_val_t *args,
    size_t nargs){
        for (size_t i = 0; i < nargs; i++)
        {
            printf("arg[%d] = %p\n", i,args[i].of.i32);
        }
}


void check_offset(){ // stack offset =  current heap address >> 32 << 32
    int var;
    int *stack_ptr = (int*)malloc(sizeof(var));
    free(stack_ptr);
    uintptr_t cur_offset = ((uint64_t)stack_ptr >> 32) << 32;

    if(MemController.offset == 0){
        // initiate
        MemController.offset = cur_offset;
    }else{
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

int main(int argc, char *argv[])
{

    wasm_engine_t *engine = wasm_engine_new();
    assert(engine != NULL);
    wasmtime_store_t *store = wasmtime_store_new(engine, NULL, NULL);
    assert(store != NULL);
    wasmtime_context_t *context = wasmtime_store_context(store);

    // Create a linker with WASI functions defined
    wasmtime_linker_t *linker = wasmtime_linker_new(engine);
    wasmtime_error_t *error = wasmtime_linker_define_wasi(linker);
    if (error != NULL)
        exit_with_error("failed to link wasi", error, NULL);

    wasm_byte_vec_t wasm;
    // Load our input file to parse it next

    if (argc != 2)
    {
        printf("Usage: %s <file_path>\n", argv[0]);
        return 1;
    }

    const char *file_path = argv[1];
    FILE *file = fopen(file_path, "rb");

    if (!file)
    {
        printf("> Error loading file!\n");
        exit(1);
    }
    fseek(file, 0L, SEEK_END);
    size_t file_size = ftell(file);
    wasm_byte_vec_new_uninitialized(&wasm, file_size);
    fseek(file, 0L, SEEK_SET);
    if (fread(wasm.data, file_size, 1, file) != 1)
    {
        printf("> Error loading module!\n");
        exit(1);
    }
    fclose(file);

    // Compile our modules
    wasmtime_module_t *module = NULL;
    error = wasmtime_module_new(engine, (uint8_t *)wasm.data, wasm.size, &module);
    if (!module)
        exit_with_error("failed to compile module", error, NULL);
    wasm_byte_vec_delete(&wasm);

    // Instantiate wasi
    wasi_config_t *wasi_config = wasi_config_new();
    assert(wasi_config);
    wasi_config_inherit_argv(wasi_config);
    wasi_config_inherit_env(wasi_config);
    wasi_config_inherit_stdin(wasi_config);
    wasi_config_inherit_stdout(wasi_config);
    wasi_config_inherit_stderr(wasi_config);
    wasm_trap_t *trap = NULL;
    error = wasmtime_context_set_wasi(context, wasi_config);
    if (error != NULL)
        exit_with_error("failed to instantiate WASI", error, NULL);

    // Create external functions
    printf("Creating callback...\n");
    register_func_to_linker(func_array, 3, linker, "env", 3);

    // Instantiate the module
    error = wasmtime_linker_module(linker, context, "", 0, module);
    if (error != NULL)
        exit_with_error("failed to instantiate module", error, NULL);

    wasmtime_instance_t instance;
    error = wasmtime_linker_instantiate(linker, context, module,
                                      &instance, NULL);
    if (error != NULL)
        exit_with_error("failed to instantiate linking1", error, trap);

    // get memory
    wasmtime_memory_t memory;
     wasmtime_func_t size_func, load_func, store_func;
    wasmtime_extern_t item;
    int ok = wasmtime_instance_export_get(context, &instance, "memory",
                                    strlen("memory"), &item);
    assert(ok && item.kind == WASMTIME_EXTERN_MEMORY);
    memory = item.of.memory;
    MemController.base = (uintptr_t)wasmtime_memory_data(context, &memory);
  // Lookup our `run` export function
    wasmtime_extern_t run;
    ok = wasmtime_instance_export_get(context, &instance, "_start", strlen("_start"), &run);
    assert(ok);
    assert(run.kind == WASMTIME_EXTERN_FUNC);
    error = wasmtime_func_call(context, &run.of.func, NULL, 0, NULL, 0, &trap);
    if (error != NULL || trap != NULL)
        exit_with_error("failed to call run", error, trap);

    return 0;
}

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap)
{
    fprintf(stderr, "error: %s\n", message);
    wasm_byte_vec_t error_message;
    if (error != NULL)
    {
        wasmtime_error_message(error, &error_message);
        wasmtime_error_delete(error);
    }
    else
    {
        wasm_trap_message(trap, &error_message);
        wasm_trap_delete(trap);
    }
    fprintf(stderr, "%.*s\n", (int)error_message.size, error_message.data);
    wasm_byte_vec_delete(&error_message);
    exit(1);
}
