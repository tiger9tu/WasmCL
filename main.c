#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasi.h>
#include <wasm.h>
#include <wasmtime.h>
#include <CL/cl.h>
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define own
static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap);

// my fake function
own wasm_trap_t *clGetPlatformIDs_callback(
    void *env, wasmtime_caller_t *caller, const wasmtime_val_t *args,
    size_t nargs, wasmtime_val_t *results, size_t nresults)
{
    cl_int CL_err = CL_SUCCESS;
    cl_uint numPlatforms = 0;

    CL_err = clGetPlatformIDs(0, NULL, &numPlatforms);

    printf("arg[0] = %u , arg[1] = %p, arg[2] = %p\n", args[0].of.i32, args[1].of.i32, args[2].of.i32);

    wasmtime_extern_t item;
    bool GE_suc = wasmtime_caller_export_get(caller, "memory", strlen("memory"), &item);
    if (GE_suc)
    {
        printf("Got export!\n");
    }
    else
    {
        printf("Failed to get export.\n");
    }

    wasmtime_memory_t memory = item.of.memory;
    printf("gt memory item! item.kind = %d\n", item.kind);
    wasmtime_context_t *context = wasmtime_caller_context(caller);
    printf("wasm memory size = %I64u\n ", wasmtime_memory_size(context, &memory));
    printf("wasm memory data size = %zd\n ", wasmtime_memory_data_size(context, &memory));
    printf("get memory of numPlatforms = %u\n", (unsigned int)(wasmtime_memory_data(context, &memory)[args[2].of.i32]));

    wasmtime_memory_data(context, &memory)[args[2].of.i32] = 2;

    if (CL_err == CL_SUCCESS)
        printf("%u platform(s) found\n", numPlatforms);
    else
        printf("clGetPlatformIDs(%i)\n", CL_err);

    results[0].of.i32 = CL_err; // clGetPlatformIDs(args[0].of.i32, args[1].of.i32, args[1].of.i32);
    return NULL;
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

    own wasm_functype_t *clGetPlatformIDs_type = wasm_functype_new_3_1(
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32(),
        wasm_valtype_new_i32());

    int i = 42;

    error = wasmtime_linker_define_func(
        linker,
        "env",
        3,
        "clGetPlatformIDs",
        strlen("clGetPlatformIDs"),
        clGetPlatformIDs_type,
        clGetPlatformIDs_callback,
        &i,
        NULL);

    wasm_functype_delete(clGetPlatformIDs_type);
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
