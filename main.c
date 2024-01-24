#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <wasi.h>
#include "mem.h"
#include "wrap.h"
#include "mem.h"

static void exit_with_error(const char *message, 
wasmtime_error_t *error,wasm_trap_t *trap);

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
    register_func_to_linker(func_array, FUNC_NUM, linker, "env", 3);

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


    MemController_init();
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

