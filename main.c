#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasi.h>
#include <wasm.h>
#include <wasmtime.h>
#include <CL/cl.h>

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap);

// // my fake function
// //  (import "env" "clGetDeviceIDs" (func $clGetDeviceIDs (type 3)))
// //   (import "env" "clCreateProgramWithSource" (func $clCreateProgramWithSource (type 4)))
// //   (import "env" "clBuildProgram" (func $clBuildProgram (type 5)))
// //   (import "env" "clGetProgramBuildInfo" (func $clGetProgramBuildInfo (type 5)))
// //   (import "env" "clCreateContext" (func $clCreateContext (type 5)))
// //   (import "env" "clCreateBuffer" (func $clCreateBuffer (type 3)))
// //   (import "env" "clCreateCommandQueue" (func $clCreateCommandQueue (type 6)))
// //   (import "env" "clCreateKernel" (func $clCreateKernel (type 1)))
// //   (import "env" "clSetKernelArg" (func $clSetKernelArg (type 7)))
// //   (import "env" "clEnqueueNDRangeKernel" (func $clEnqueueNDRangeKernel (type 8)))
// //   (import "env" "clEnqueueReadBuffer" (func $clEnqueueReadBuffer (type 8)))
// //   (import "env" "clReleaseKernel" (func $clReleaseKernel (type 0)))
// //   (import "env" "clReleaseMemObject" (func $clReleaseMemObject (type 0)))
// //   (import "env" "clReleaseCommandQueue" (func $clReleaseCommandQueue (type 0)))
// //   (import "env" "clReleaseProgram" (func $clReleaseProgram (type 0)))
// //   (import "env" "clReleaseContext" (func $clReleaseContext (type 0)))

// WASM_API_EXTERN wasmtime_error_t *wasmtime_linker_define_func(
//     wasmtime_linker_t *linker, const char *module, size_t module_len,
//     const char *name, size_t name_len, const wasm_functype_t *ty,
//     wasmtime_func_callback_t cb, void *data, void (*finalizer)(void *));

// static inline wasm_functype_t *wasm_functype_new_3_1(
//     wasm_valtype_t *p1, wasm_valtype_t *p2, wasm_valtype_t *p3,
//     wasm_valtype_t *r)
// {
//     wasm_valtype_t *ps[3] = {p1, p2, p3};
//     wasm_valtype_t *rs[1] = {r};
//     wasm_valtype_vec_t params, results;
//     wasm_valtype_vec_new(&params, 3, ps);
//     wasm_valtype_vec_new(&results, 1, rs);
//     return wasm_functype_new(&params, &results);
// }

wasm_trap_t *clGetPlatformIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    cl_int CL_err = CL_SUCCESS;
    cl_uint numPlatforms = 0;
    CL_err = clGetPlatformIDs(args[0].of.i32, args[1].of.i32, &numPlatforms);

    wasmtime_extern_t item;
    bool GE_suc = wasmtime_caller_export_get(caller, "memory", strlen("memory"), &item);
    assert(GE_suc);

    wasmtime_memory_t memory = item.of.memory;
    wasmtime_context_t *context = wasmtime_caller_context(caller);
    wasmtime_memory_data(context, &memory)[args[2].of.i32] = numPlatforms;

    return NULL;
}

#define MAX_CL_NAME 30
#define MAX_CL_PARAM 8

typedef struct
{
    char name[MAX_CL_NAME];
    size_t name_len;

    wasm_valkind_t param_types[MAX_CL_PARAM];
    size_t param_len;

    wasm_valkind_t result_type;
    wasmtime_func_async_callback_t cb;

} define_func;

define_func func_array[1] = {
    {.name = "clGetPlatformIDs",
     .name_len = 16, // this is a problem
     .param_types = {
         WASM_I32, WASM_I32, WASM_I32},
     .param_len = 3,
     .result_type = WASM_I32,
     .cb = clGetPlatformIDs_callback}};

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
            tmp.name_len,
            tmp_func_type,
            tmp.cb,
            &i,
            NULL);

        wasm_functype_delete(tmp_func_type);
    }
}

int main(int argc, char *argv[])
{

    // Set up our context
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
    register_func_to_linker(func_array, 1, linker, "env", 3);

    // Instantiate the module
    error = wasmtime_linker_module(linker, context, "", 0, module);
    if (error != NULL)
        exit_with_error("failed to instantiate module", error, NULL);

    // Run it.
    wasmtime_func_t func;
    error = wasmtime_linker_get_default(linker, context, "", 0, &func);
    if (error != NULL)
        exit_with_error("failed to locate default export for module", error, NULL);

    error = wasmtime_func_call(context, &func, NULL, 0, NULL, 0, &trap);
    if (error != NULL || trap != NULL)
        exit_with_error("error calling default export", error, trap);

    // Clean up after ourselves at this point
    wasmtime_module_delete(module);
    wasmtime_store_delete(store);
    wasm_engine_delete(engine);
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
