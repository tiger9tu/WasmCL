#ifndef WRAP_H
#define WRAP_H

#include <CL/cl.h>
#include <wasm.h>
#include <wasmtime.h>

#define MAX_CL_NAME 30
#define MAX_CL_PARAM 12

typedef struct {
  wasm_valkind_t param_types[MAX_CL_PARAM];
  size_t param_len;
  wasm_valkind_t result_type;
} func_type;

typedef struct {
  char name[MAX_CL_NAME];
  wasmtime_func_async_callback_t cb;
  func_type ft;
} define_func;

#define FUNC_NUM 20

int register_func_to_linker(define_func funcs[], int count,
                            wasmtime_linker_t *linker, const char *module,
                            size_t module_len);

extern define_func func_array[];
extern cl_mem cl_buffer; // TODO: remove

#endif // WRAP_H
