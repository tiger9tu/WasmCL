#include "mem.h"
#include "wrap.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <wasi.h>

const char *file_path;
char folder_path[256];

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap);

int main(int argc, char *argv[]) {

  // for initiate time checking
  clock_t start, end;
  double wasm_compile_time, wasm_init_time, wasmtime_start_time,
      wasmtime_runtime_and_wasm_runtime, wasmtime_runtime;

  start = clock();

  // Load our input file to parse it next
  wasm_byte_vec_t wasm;
  if (argc != 2) {
    printf("Usage: %s <file_path>\n", argv[0]);
    return 1;
  }

  file_path = argv[1];
  FILE *file = fopen(file_path, "rb");

  printf("file: %s\n", file_path);

  if (!file) {
    printf("> Error loading file!\n");
    exit(1);
  }
  fseek(file, 0L, SEEK_END);
  size_t file_size = ftell(file);
  wasm_byte_vec_new_uninitialized(&wasm, file_size);
  fseek(file, 0L, SEEK_SET);
  if (fread(wasm.data, file_size, 1, file) != 1) {
    printf("> Error loading module!\n");
    exit(1);
  }
  fclose(file);

  wasm_engine_t *engine = wasm_engine_new();
  assert(engine != NULL);
  wasm_store_t *store = wasmtime_store_new(engine, NULL, NULL);
  assert(store != NULL);
  wasmtime_context_t *context = wasmtime_store_context(store);

  // Create a linker with WASI functions defined
  wasmtime_linker_t *linker = wasmtime_linker_new(engine);
  wasmtime_error_t *error = wasmtime_linker_define_wasi(linker);
  if (error != NULL)
    exit_with_error("failed to link wasi", error, NULL);

  // Instantiate wasi
  wasi_config_t *wasi_config = wasi_config_new();
  assert(wasi_config);
  wasi_config_inherit_argv(wasi_config);
  wasi_config_inherit_env(wasi_config);
  wasi_config_inherit_stdin(wasi_config);
  wasi_config_inherit_stdout(wasi_config);
  wasi_config_inherit_stderr(wasi_config);

  const char *last_slash = strrchr(file_path, '/');
  if (last_slash != NULL) {
    strncpy(folder_path, file_path, last_slash - file_path);
    folder_path[last_slash - file_path] = '\0';
  } else {
    strcpy(folder_path, ".");
  }
  wasi_config_preopen_dir(wasi_config, folder_path, ".");
  wasm_trap_t *trap = NULL;
  error = wasmtime_context_set_wasi(context, wasi_config);
  if (error != NULL)
    exit_with_error("failed to instantiate WASI", error, NULL);

  // Create external functions
  printf("Creating callback...\n");
  register_func_to_linker(func_array, FUNC_NUM, linker, "env", 3);

  end = clock();
  wasmtime_start_time = ((double)(end - start));

  // Compile our modules
  start = clock();
  wasmtime_module_t *module = NULL;
  error = wasmtime_module_new(engine, (uint8_t *)wasm.data, wasm.size, &module);
  if (!module)
    exit_with_error("failed to compile module", error, NULL);
  wasm_byte_vec_delete(&wasm);
  end = clock();
  wasm_compile_time = ((double)(end - start));

  // Instantiate the module
  start = clock();
  error = wasmtime_linker_module(linker, context, "", 0, module);
  if (error != NULL)
    exit_with_error("failed to instantiate module", error, NULL);

  wasmtime_instance_t instance;
  error = wasmtime_linker_instantiate(linker, context, module, &instance, NULL);
  if (error != NULL)
    exit_with_error("failed to instantiate linking1", error, trap);
  end = clock();
  wasm_init_time = ((double)(end - start));

  // get memory
  wasmtime_memory_t memory;
  wasmtime_func_t size_func, load_func, store_func;
  wasmtime_extern_t item;
  int ok = wasmtime_instance_export_get(context, &instance, "memory",
                                        strlen("memory"), &item);
  assert(ok && item.kind == WASMTIME_EXTERN_MEMORY);
  memory = item.of.memory;

  MemController.base = (uintptr_t)wasmtime_memory_data(context, &memory);
  initHashMap(&MemController.clAddrRec);

  // Lookup our `run` export function
  wasmtime_extern_t run;
  ok = wasmtime_instance_export_get(context, &instance, "_start",
                                    strlen("_start"), &run);
  assert(ok);
  assert(run.kind == WASMTIME_EXTERN_FUNC);

  start = clock();
  error = wasmtime_func_call(context, &run.of.func, NULL, 0, NULL, 0, &trap);
  end = clock();
  wasmtime_runtime_and_wasm_runtime = ((double)(end - start));

  if (error != NULL || trap != NULL)
    exit_with_error("failed to call run", error, trap);

  // output time
  printf("\nTime results:\n\n"
         "file size: %dBytes\n"
         "Compile module time: %fms\n"
         "Initiate module time:%fms\n"
         "wasmtime_start_time: %fms\n"
         "wasmtime_runtime_and_wasm_runtime: %fms\n\n\n\n",
         file_size, wasm_compile_time, wasm_init_time, wasmtime_start_time,
         wasmtime_runtime_and_wasm_runtime);

  return 0;
}

static void exit_with_error(const char *message, wasmtime_error_t *error,
                            wasm_trap_t *trap) {
  fprintf(stderr, "error: %s\n", message);
  wasm_byte_vec_t error_message;
  if (error != NULL) {
    wasmtime_error_message(error, &error_message);
    wasmtime_error_delete(error);
  } else {
    wasm_trap_message(trap, &error_message);
    wasm_trap_delete(trap);
  }
  fprintf(stderr, "%.*s\n", (int)error_message.size, error_message.data);
  wasm_byte_vec_delete(&error_message);
  exit(1);
}
