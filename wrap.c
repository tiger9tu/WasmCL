#include "wrap.h"
#include "mem.h"
#include <assert.h>
#include <stdio.h>

void puts_(const char *str) {
  // puts(str);
}

int register_func_to_linker(define_func funcs[], int count,
                            wasmtime_linker_t *linker, const char *module,
                            size_t module_len) {
  for (size_t i = 0; i < count; i++) {
    define_func tmp = funcs[i];

    wasm_valtype_t *params[MAX_CL_PARAM];
    for (size_t j = 0; j < tmp.ft.param_len; j++) {
      params[j] = wasm_valtype_new(tmp.ft.param_types[j]);
    }

    wasm_valtype_t *result[1] = {wasm_valtype_new(tmp.ft.result_type)};

    wasm_valtype_vec_t params_vec, result_vec;
    wasm_valtype_vec_new(&params_vec, tmp.ft.param_len, params);
    wasm_valtype_vec_new(&result_vec, 1 /*in c there is only 1 result*/,
                         result);

    wasm_functype_t *tmp_func_type =
        wasm_functype_new(&params_vec, &result_vec);

    int i = 42;

    wasmtime_error_t *error = wasmtime_linker_define_func(
        linker, "env", 3, tmp.name, strlen(tmp.name), tmp_func_type, tmp.cb, &i,
        NULL);

    wasm_functype_delete(tmp_func_type);
  }
}

void print_args(const wasmtime_val_t *args, size_t nargs) {
  // for (size_t i = 0; i < nargs; i++) {
  //   printf("arg%d = %p\n", i, args[i].of.i32);
  // }
}

void print_args_(const wasmtime_val_t *args, size_t nargs) {
  // for (size_t i = 0; i < nargs; i++) {
  //   printf("arg%d = %p\n", i, args[i].of.i32);
  // }
}

// platform layer

wasm_trap_t *clGetPlatformIDs_callback(void *env, wasmtime_caller_t *caller,
                                       const wasmtime_val_t *args, size_t nargs,
                                       wasmtime_val_t *results,
                                       size_t nresults) {

  puts_("calling clGetPlatformIDS");
  print_args(args, nargs);

  intptr_t buffer[MAX_PTR_DEPTH];
  intptr_t *addr64_arg_platforms = get_addr64_arg_w(
      buffer, get_host_addr(args[1].of.i32, WASM), args[0].of.i32, 1);

  results[0].of.i32 = clGetPlatformIDs(args[0].of.i32, addr64_arg_platforms,
                                       get_host_addr(args[2].of.i32, WASM));

  cp_host_addr_to_wasm(get_host_addr(args[1].of.i32, WASM),
                       addr64_arg_platforms, args[0].of.i32);

  return NULL;
}

wasm_trap_t *clGetPlatformInfo_callback(void *env, wasmtime_caller_t *caller,
                                        const wasmtime_val_t *args,
                                        size_t nargs, wasmtime_val_t *results,
                                        size_t nresults) {

  puts_("calling clGetPlatformInfo");
  results[0].of.i32 = clGetPlatformInfo(
      get_host_addr(args[0].of.i32, TRUNC), args[1].of.i32, args[2].of.i32,
      get_host_addr(args[3].of.i32, WASM), get_host_addr(args[4].of.i32, WASM));

  return NULL;
}

wasm_trap_t *clGetDeviceIDs_callback(void *env, wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args, size_t nargs,
                                     wasmtime_val_t *results, size_t nresults) {
  puts_("calling getDeviceIDS");
  print_args(args, nargs);

  cl_uint num_entries = args[2].of.i32;
  uint32_t *devices = (cl_device_id *)get_host_addr(args[3].of.i32, WASM);

  intptr_t buffer[MAX_PTR_DEPTH];
  intptr_t *addr64_arg_devices =
      get_addr64_arg_w(buffer, devices, num_entries, 1);

  results[0].of.i32 = clGetDeviceIDs(
      get_host_addr(args[0].of.i32, TRUNC), args[1].of.i64, args[2].of.i32,
      addr64_arg_devices, get_host_addr(args[4].of.i32, WASM));

  cp_host_addr_to_wasm(devices, addr64_arg_devices, num_entries);

  return NULL;
}

wasm_trap_t *clGetDeviceInfo_callback(void *env, wasmtime_caller_t *caller,
                                      const wasmtime_val_t *args, size_t nargs,
                                      wasmtime_val_t *results,
                                      size_t nresults) {

  puts_("calling clGetDeviceInfo");
  results[0].of.i32 =
      clGetDeviceInfo(get_host_addr(args[0].of.i32, TRUNC), args[1].of.i32,
                      args[2].of.i32, get_host_addr(args[3].of.i32, WASM),
                      get_host_addr_auto(args[4].of.i32, WASM));

  return NULL;
}

wasm_trap_t *clCreateContext_callback(void *env, wasmtime_caller_t *caller,
                                      const wasmtime_val_t *args, size_t nargs,
                                      wasmtime_val_t *results,
                                      size_t nresults) {
  check_offset();
  puts_("calling clCreateContext");
  print_args(args, nargs);

  cl_uint num_devices = args[1].of.i32;
  uint32_t *devices = get_host_addr(args[2].of.i32, WASM);
  uintptr_t buffer[MAX_PTR_DEPTH];
  uintptr_t *addr64_arg_devices =
      get_addr64_arg_r(buffer, devices, TRUNC, num_devices, 1);

  cl_context res = clCreateContext(
      get_host_addr(args[0].of.i32, WASM), num_devices, addr64_arg_devices,
      get_host_addr(args[3].of.i32, WASM), get_host_addr(args[4].of.i32, WASM),
      get_host_addr(args[5].of.i32, WASM));

  cp_host_addr_to_wasm(&results[0].of.i32, &res, 1);
  return NULL;
}

wasm_trap_t *clReleaseContext_callback(void *env, wasmtime_caller_t *caller,
                                       const wasmtime_val_t *args, size_t nargs,
                                       wasmtime_val_t *results,
                                       size_t nresults) {

  puts_("calling clReleaseContext");
  results[0].of.i32 = clReleaseContext(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

// runtime
wasm_trap_t *clCreateCommandQueue_callback(void *env, wasmtime_caller_t *caller,
                                           const wasmtime_val_t *args,
                                           size_t nargs,
                                           wasmtime_val_t *results,
                                           size_t nresults) {

  puts_("calling clCreateCommandQueue");
  print_args(args, nargs);
  cl_command_queue res =
      clCreateCommandQueue(get_host_addr(args[0].of.i32, TRUNC),
                           get_host_addr(args[1].of.i32, TRUNC), args[2].of.i64,
                           get_host_addr(args[3].of.i32, WASM));
  cp_host_addr_to_wasm(&results[0].of.i32, &res, 1);
  return NULL;
}

wasm_trap_t *
clReleaseCommandQueue_callback(void *env, wasmtime_caller_t *caller,
                               const wasmtime_val_t *args, size_t nargs,
                               wasmtime_val_t *results, size_t nresults) {

  puts_("calling clReleaseCommandQueue");
  results[0].of.i32 = clReleaseCommandQueue(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

// Buffer Objects
cl_mem cl_memdebug;
wasm_trap_t *clCreateBuffer_callback(void *env, wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args, size_t nargs,
                                     wasmtime_val_t *results, size_t nresults) {

  puts_("calling clCreateBuffer");
  print_args(args, nargs);
  cl_mem res = clCreateBuffer(
      get_host_addr(args[0].of.i32, TRUNC), args[1].of.i64, args[2].of.i32,
      get_host_addr(args[3].of.i32, WASM), get_host_addr(args[4].of.i32, WASM));

  cp_host_addr_to_wasm(&results[0].of.i32, &res, 1);
  return NULL;
}

wasm_trap_t *clEnqueueReadBuffer_callback(void *env, wasmtime_caller_t *caller,
                                          const wasmtime_val_t *args,
                                          size_t nargs, wasmtime_val_t *results,
                                          size_t nresults) {

  puts_("calling clEnqueueReadBuffer");

  intptr_t buffer_1[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event_wait_list = get_addr64_arg_r(
      buffer_1, get_host_addr_auto(args[7].of.i32), TRUNC, args[6].of.i32, 1);

  intptr_t buffer_2[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event =
      get_addr64_arg_w(buffer_2, get_host_addr_auto(args[8].of.i32), 1, 1);

  results[0].of.i32 = clEnqueueReadBuffer(
      get_host_addr(args[0].of.i32, TRUNC),
      get_host_addr_auto(args[1].of.i32, TRUNC), args[2].of.i32, args[3].of.i32,
      args[4].of.i32, get_host_addr_auto(args[5].of.i32, WASM), args[6].of.i32,
      addr64_arg_event_wait_list, addr64_arg_event);

  return NULL;
}

wasm_trap_t *clEnqueueWriteBuffer_callback(void *env, wasmtime_caller_t *caller,
                                           const wasmtime_val_t *args,
                                           size_t nargs,
                                           wasmtime_val_t *results,
                                           size_t nresults) {

  puts_("calling clEnqueueWriteBuffer");
  print_args(args, nargs);

  intptr_t buffer_1[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event_wait_list = get_addr64_arg_r(
      buffer_1, get_host_addr_auto(args[7].of.i32), TRUNC, args[6].of.i32, 1);

  intptr_t buffer_2[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event =
      get_addr64_arg_w(buffer_2, get_host_addr_auto(args[8].of.i32), 1, 1);

  // debug
  float *a = get_host_addr_auto(args[5].of.i32);

  results[0].of.i32 = clEnqueueWriteBuffer(
      get_host_addr(args[0].of.i32, TRUNC),
      get_host_addr(args[1].of.i32, TRUNC), args[2].of.i32, args[3].of.i32,
      args[4].of.i32, get_host_addr(args[5].of.i32, WASM), args[6].of.i32,
      addr64_arg_event_wait_list, addr64_arg_event);

  return NULL;
}

// Can not implement this function,
// because wasm can not read the result data in host

// wasm_trap_t *clEnqueueMapBuffer_callback(void *env, wasmtime_caller_t
// *caller,
//                                          const wasmtime_val_t *args,
//                                          size_t nargs, wasmtime_val_t
//                                          *results, size_t nresults) {
//   puts_("calling clEnqueueMapBuffer");
//   intptr_t buffer_1[MAX_PTR_DEPTH];
//   intptr_t addr64_arg_event_wait_list = get_addr64_arg_r(
//       buffer_1, get_host_addr_auto(args[7].of.i32), TRUNC, args[6].of.i32,
//       1);

//   intptr_t buffer_2[MAX_PTR_DEPTH];
//   intptr_t addr64_arg_event =
//       get_addr64_arg_w(buffer_2, get_host_addr_auto(args[8].of.i32), 1, 1);

//   void *resPtr = clEnqueueMapBuffer(
//       get_host_addr_auto(args[0].of.i32), get_host_addr_auto(args[1].of.i32),
//       args[2].of.i32, args[3].of.i64, args[4].of.i32, args[5].of.i32,
//       args[6].of.i32, addr64_arg_event_wait_list, addr64_arg_event,
//       get_host_addr_auto(args[9].of.i32));

//   printf("resPtr = %p\n", resPtr);
//   results[0].of.i32 = resPtr;
//   printf("after calling clenqueueMapBuffer\n");
//   return NULL;
// }

// Memory Objects

wasm_trap_t *clReleaseMemObject_callback(void *env, wasmtime_caller_t *caller,
                                         const wasmtime_val_t *args,
                                         size_t nargs, wasmtime_val_t *results,
                                         size_t nresults) {

  puts_("calling clReleaseMemObject");
  results[0].of.i32 = clReleaseMemObject(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

wasm_trap_t *
clEnqueueUnmapMemObject_callback(void *env, wasmtime_caller_t *caller,
                                 const wasmtime_val_t *args, size_t nargs,
                                 wasmtime_val_t *results, size_t nresults) {

  puts_("calling clEnqueueUnmapMemObject");
  intptr_t buffer_1[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event_wait_list = get_addr64_arg_r(
      buffer_1, get_host_addr_auto(args[7].of.i32), TRUNC, args[6].of.i32, 1);

  results[0].of.i32 = clEnqueueUnmapMemObject(
      get_host_addr_auto(args[0].of.i32), get_host_addr_auto(args[1].of.i32),
      get_host_addr_auto(args[2].of.i32), args[3].of.i32,
      addr64_arg_event_wait_list, get_host_addr_auto(args[4].of.i32));
  return NULL;
}

// Program Objects

wasm_trap_t *
clCreateProgramWithSource_callback(void *env, wasmtime_caller_t *caller,
                                   const wasmtime_val_t *args, size_t nargs,
                                   wasmtime_val_t *results, size_t nresults) {
  check_offset();
  puts_("calling clCreateProgramWithSource");
  print_args(args, nargs);

  uintptr_t buffer[MAX_PTR_DEPTH];
  uintptr_t *addr64_arg_strings = get_addr64_arg_r(
      buffer, get_host_addr(args[2].of.i32, WASM), WASM, args[1].of.i32, 1);

  char **strings = addr64_arg_strings;

  size_t lengths[MAX_PROGRAM_COUNT];
  int count = args[1].of.i32;
  unsigned int *tmp;
  tmp = get_host_addr(args[3].of.i32, WASM);
  for (int i = 0; i < count; i++) {
    lengths[i] = (size_t)tmp[i];
  }

  cl_program res = clCreateProgramWithSource(
      get_host_addr(args[0].of.i32, TRUNC), args[1].of.i32, addr64_arg_strings,
      lengths, get_host_addr(args[4].of.i32, WASM));
  cp_host_addr_to_wasm(&results[0].of.i32, &res, 1);
  return NULL;
}

wasm_trap_t *
clCreateProgramWithIL_callback(void *env, wasmtime_caller_t *caller,
                               const wasmtime_val_t *args, size_t nargs,
                               wasmtime_val_t *results, size_t nresults) {

  puts_("calling clCreateProgramWithIL");
  cl_program res = clCreateProgramWithIL(
      get_host_addr_auto(args[0].of.i32), get_host_addr_auto(args[1].of.i32),
      args[2].of.i32, get_host_addr_auto(args[3].of.i32));
  cp_host_addr_to_wasm(&results[0].of.i32, &res, 1);
  return NULL;
}

wasm_trap_t *clGetProgramInfo_callback(void *env, wasmtime_caller_t *caller,
                                       const wasmtime_val_t *args, size_t nargs,
                                       wasmtime_val_t *results,
                                       size_t nresults) {

  puts_("calling clGetProgramInfo");
  results[0].of.i32 = clGetProgramInfo(
      get_host_addr_auto(args[0].of.i32), args[1].of.i32, args[2].of.i32,
      get_host_addr_auto(args[3].of.i32), get_host_addr_auto(args[4].of.i32));

  return NULL;
}

wasm_trap_t *clReleaseProgram_callback(void *env, wasmtime_caller_t *caller,
                                       const wasmtime_val_t *args, size_t nargs,
                                       wasmtime_val_t *results,
                                       size_t nresults) {

  puts_("calling clReleaseProgram");
  results[0].of.i32 = clReleaseProgram(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

wasm_trap_t *clBuildProgram_callback(void *env, wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args, size_t nargs,
                                     wasmtime_val_t *results, size_t nresults) {

  puts_("calling clBuildProgram");
  print_args(args, nargs);
  intptr_t buffer[MAX_PTR_DEPTH];
  intptr_t *addr64_arg_device_list = get_addr64_arg_r(
      buffer, get_host_addr(args[2].of.i32, WASM), TRUNC, args[1].of.i32, 1);

  results[0].of.i32 = clBuildProgram(
      get_host_addr(args[0].of.i32, TRUNC), args[1].of.i32,
      addr64_arg_device_list, get_host_addr(args[3].of.i32, WASM),
      get_host_addr(args[4].of.i32, WASM), get_host_addr(args[5].of.i32, WASM));

  return NULL;
}

wasm_trap_t *
clGetProgramBuildInfo_callback(void *env, wasmtime_caller_t *caller,
                               const wasmtime_val_t *args, size_t nargs,
                               wasmtime_val_t *results, size_t nresults) {

  results[0].of.i32 = clGetProgramBuildInfo(
      get_host_addr(args[0].of.i32, WASM), get_host_addr(args[1].of.i32, TRUNC),
      args[2].of.i32, args[3].of.i32, get_host_addr(args[4].of.i32, WASM),
      get_host_addr(args[5].of.i32, WASM));

  return NULL;
}

// Kernel Objects

wasm_trap_t *clCreateKernel_callback(void *env, wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args, size_t nargs,
                                     wasmtime_val_t *results, size_t nresults) {

  puts_("calling clCreateKernel");
  print_args(args, nargs);
  cl_kernel res = clCreateKernel(get_host_addr(args[0].of.i32, TRUNC),
                                 get_host_addr(args[1].of.i32, WASM),
                                 get_host_addr(args[2].of.i32, WASM));

  cp_host_addr_to_wasm(&results[0].of.i32, &res, 1);
  return NULL;
}

wasm_trap_t *clReleaseKernel_callback(void *env, wasmtime_caller_t *caller,
                                      const wasmtime_val_t *args, size_t nargs,
                                      wasmtime_val_t *results,
                                      size_t nresults) {

  puts_("calling clReleaseKernel");
  results[0].of.i32 = clReleaseKernel(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

bool is_ptr(uint32_t wasm_value) { return (wasm_value >> 20) > 0; }

wasm_trap_t *clSetKernelArg_callback(void *env, wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args, size_t nargs,
                                     wasmtime_val_t *results, size_t nresults) {

  puts_("calling clSetKernelArg");
  print_args(args, nargs);

  intptr_t buffer[MAX_PTR_DEPTH];
  intptr_t *addr64_arg_value = get_addr64_arg_r(
      buffer, get_host_addr(args[3].of.i32, WASM), TRUNC, 1, 1);

  size_t arg_size = args[2].of.i32;

  // 判断是否是地址形参数，如果是的话就要将size * 2
  uint32_t *arg_value_target = (void **)get_host_addr(args[3].of.i32, WASM);

  if (get_mem_type(*arg_value_target) == TRUNC) {
    arg_size *= 2;
  }

  results[0].of.i32 =
      clSetKernelArg(get_host_addr(args[0].of.i32, TRUNC), args[1].of.i32,
                     arg_size, addr64_arg_value);

  return NULL;
}

wasm_trap_t *
clEnqueueNDRangeKernel_callback(void *env, wasmtime_caller_t *caller,
                                const wasmtime_val_t *args, size_t nargs,
                                wasmtime_val_t *results, size_t nresults) {

  puts_("calling clEnqueueNDRangeKernel");
  print_args_(args, nargs);
  intptr_t buffer_1[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event_wait_list = get_addr64_arg_r(
      buffer_1, get_host_addr_auto(args[7].of.i32), TRUNC, args[6].of.i32, 1);
  intptr_t buffer_2[MAX_PTR_DEPTH];
  intptr_t addr64_arg_event =
      get_addr64_arg_w(buffer_2, get_host_addr_auto(args[8].of.i32), 1, 1);
  // size_t 32 64位兼容问题

  size_t *global_work_offset_ptr = NULL, *global_work_size_ptr = NULL,
         *local_work_size_ptr = NULL;

  size_t global_work_offset_buffer[MAX_PTR_DEPTH],
      global_work_size_buffer[MAX_PTR_DEPTH],
      local_work_size_buffer[MAX_PTR_DEPTH];

  cl_uint work_dim = args[2].of.i32;
  puts_("BEFORE 3");
  if (args[3].of.i32 != NULL) {
    cp_wasm_size_t_to_host((uint32_t *)(get_host_addr_auto(args[3].of.i32)),
                           global_work_offset_buffer, work_dim);
    global_work_offset_ptr = global_work_offset_buffer;
  }
  puts_("AFTER 3 BEFORE 4");
  if (args[4].of.i32 != NULL) {
    cp_wasm_size_t_to_host((uint32_t *)(get_host_addr_auto(args[4].of.i32)),
                           global_work_size_buffer, work_dim);
    global_work_size_ptr = global_work_size_buffer;
  }
  puts_("AFTER 4 BEFORE 5");
  if (args[5].of.i32 != NULL) {
    cp_wasm_size_t_to_host((uint32_t *)(get_host_addr_auto(args[5].of.i32)),
                           local_work_size_buffer, work_dim);
    local_work_size_ptr = local_work_size_buffer;
  }
  puts_("before clenqueueNDRANGE");
  results[0].of.i32 = clEnqueueNDRangeKernel(
      get_host_addr_auto(args[0].of.i32), get_host_addr_auto(args[1].of.i32),
      args[2].of.i32, global_work_offset_ptr, global_work_size_ptr,
      local_work_size_ptr, args[6].of.i32, addr64_arg_event_wait_list,
      addr64_arg_event);

  cp_host_addr_to_wasm(get_host_addr_auto(args[8].of.i32), addr64_arg_event, 1);

  return NULL;
}

// Event Objects
wasm_trap_t *clEnqueueBarrier_callback(void *env, wasmtime_caller_t *caller,
                                       const wasmtime_val_t *args, size_t nargs,
                                       wasmtime_val_t *results,
                                       size_t nresults) {

  puts_("calling clEnqueueBarrier");
  print_args(args, nargs);
  results[0].of.i32 = clEnqueueBarrier(get_host_addr(args[0].of.i32, TRUNC));
  return NULL;
}
// Other functions
wasm_trap_t *clFinish_callback(void *env, wasmtime_caller_t *caller,
                               const wasmtime_val_t *args, size_t nargs,
                               wasmtime_val_t *results, size_t nresults) {

  puts_("calling clFinish");
  print_args(args, nargs);

  results[0].of.i32 = clFinish(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

wasm_trap_t *clFlush_callback(void *env, wasmtime_caller_t *caller,
                              const wasmtime_val_t *args, size_t nargs,
                              wasmtime_val_t *results, size_t nresults) {

  puts_("calling clFlush");
  print_args(args, nargs);

  results[0].of.i32 = clFlush(get_host_addr_auto(args[0].of.i32));

  return NULL;
}

extern void get_wasm_section(const char *c_file_name, const char *c_sec_name,
                             char *c_ret_buf, int *c_sec_len);
extern const char *file_path;
wasm_trap_t *
clExtGetPreCompiledILOfFile_callback(void *env, wasmtime_caller_t *caller,
                                     const wasmtime_val_t *args, size_t nargs,
                                     wasmtime_val_t *results, size_t nresults) {

  puts_("calling clExtGetPreCompiledILOfFile");
  print_args(args, nargs);

  get_wasm_section(file_path, "SPIRV", get_host_addr_auto(args[0].of.i32),
                   get_host_addr_auto(args[1].of.i32));

  results[0].of.i32 = CL_SUCCESS; // useless

  return NULL;
}

// common function types:
#define COMMON_FUNC_TYPE1                                                      \
  { .param_types = {WASM_I32}, .param_len = 1, .result_type = WASM_I32 }
#define COMMON_FUNC_TYPE2                                                      \
  {                                                                            \
    .param_types = {WASM_I32, WASM_I32}, .param_len = 2,                       \
    .result_type = WASM_I32                                                    \
  }
#define COMMON_FUNC_TYPE3                                                      \
  {                                                                            \
    .param_types = {WASM_I32, WASM_I32, WASM_I32}, .param_len = 3,             \
    .result_type = WASM_I32                                                    \
  }
#define COMMON_FUNC_TYPE4                                                      \
  {                                                                            \
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32}, .param_len = 4,   \
    .result_type = WASM_I32                                                    \
  }
#define COMMON_FUNC_TYPE5                                                      \
  {                                                                            \
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},         \
    .param_len = 5, .result_type = WASM_I32                                    \
  }
#define COMMON_FUNC_TYPE6                                                      \
  {                                                                            \
    .param_types =                                                             \
        {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32},          \
    .param_len = 6, .result_type = WASM_I32                                    \
  }
#define COMMON_FUNC_TYPE9                                                      \
  {                                                                            \
    .param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I32, WASM_I32,          \
                    WASM_I32, WASM_I32, WASM_I32, WASM_I32},                   \
    .param_len = 9, .result_type = WASM_I32                                    \
  }

define_func func_array[30] = {
    // Platform Layer
    {.name = "clGetPlatformIDs",
     .cb = clGetPlatformIDs_callback,
     .ft = COMMON_FUNC_TYPE3},
    {.name = "clGetPlatformInfo",
     .cb = clGetPlatformInfo_callback,
     .ft = COMMON_FUNC_TYPE5},
    {.name = "clGetDeviceIDs",
     .cb = clGetDeviceIDs_callback,
     .ft = {.param_types = {WASM_I32, WASM_I64, WASM_I32, WASM_I32, WASM_I32},
            .param_len = 5,
            .result_type = WASM_I32}},
    {.name = "clGetDeviceInfo",
     .cb = clGetDeviceInfo_callback,
     .ft = COMMON_FUNC_TYPE5},
    {.name = "clCreateContext",
     .cb = clCreateContext_callback,
     .ft = COMMON_FUNC_TYPE6},
    {.name = "clReleaseContext",
     .cb = clReleaseContext_callback,
     .ft = COMMON_FUNC_TYPE1},
    {.name = "clCreateCommandQueue",
     .cb = clCreateCommandQueue_callback,
     .ft = {.param_types = {WASM_I32, WASM_I32, WASM_I64, WASM_I32},
            .param_len = 4,
            .result_type = WASM_I32}},
    {.name = "clReleaseCommandQueue",
     .cb = clReleaseCommandQueue_callback,
     .ft = COMMON_FUNC_TYPE1},
    // Buffer Objects
    {.name = "clCreateBuffer",
     .cb = clCreateBuffer_callback,
     .ft = {.param_types = {WASM_I32, WASM_I64, WASM_I32, WASM_I32, WASM_I32},
            .param_len = 5,
            .result_type = WASM_I32}},
    {.name = "clEnqueueReadBuffer",
     .cb = clEnqueueReadBuffer_callback,
     .ft = COMMON_FUNC_TYPE9},
    {.name = "clEnqueueWriteBuffer",
     .cb = clEnqueueWriteBuffer_callback,
     .ft = COMMON_FUNC_TYPE9},
    // {.name = "clEnqueueMapBuffer",
    //  .cb = clEnqueueMapBuffer_callback,
    //  .ft = {.param_types = {WASM_I32, WASM_I32, WASM_I32, WASM_I64, WASM_I32,
    //                         WASM_I32, WASM_I32, WASM_I32, WASM_I32,
    //                         WASM_I32},
    //         .param_len = 10,
    //         .result_type = WASM_I32}},
    // Memory Objects
    {.name = "clReleaseMemObject",
     .cb = clReleaseMemObject_callback,
     .ft = COMMON_FUNC_TYPE1},
    {.name = "clEnqueueUnmapMemObject",
     .cb = clEnqueueUnmapMemObject_callback,
     .ft = COMMON_FUNC_TYPE6},
    // Program Objects
    {.name = "clCreateProgramWithSource",
     .cb = clCreateProgramWithSource_callback,
     .ft = COMMON_FUNC_TYPE5},
    {.name = "clCreateProgramWithIL",
     .cb = clCreateProgramWithIL_callback,
     .ft = COMMON_FUNC_TYPE4},
    {.name = "clGetProgramInfo",
     .cb = clGetProgramInfo_callback,
     .ft = COMMON_FUNC_TYPE5},
    {.name = "clReleaseProgram",
     .cb = clReleaseProgram_callback,
     .ft = COMMON_FUNC_TYPE1},
    {.name = "clBuildProgram",
     .cb = clBuildProgram_callback,
     .ft = COMMON_FUNC_TYPE6},
    {.name = "clGetProgramBuildInfo",
     .cb = clGetProgramBuildInfo_callback,
     .ft = COMMON_FUNC_TYPE6},
    // Kernel Objects
    {.name = "clCreateKernel",
     .cb = clCreateKernel_callback,
     .ft = COMMON_FUNC_TYPE3},
    {.name = "clReleaseKernel",
     .cb = clReleaseKernel_callback,
     .ft = COMMON_FUNC_TYPE1},
    {.name = "clSetKernelArg",
     .cb = clSetKernelArg_callback,
     .ft = COMMON_FUNC_TYPE4},
    {.name = "clEnqueueNDRangeKernel",
     .cb = clEnqueueNDRangeKernel_callback,
     .ft = COMMON_FUNC_TYPE9},
    // Event
    {.name = "clEnqueueBarrier",
     .cb = clEnqueueBarrier_callback,
     .ft = COMMON_FUNC_TYPE1},
    // Others
    {.name = "clFinish", .cb = clFinish_callback, .ft = COMMON_FUNC_TYPE1},
    {.name = "clFlush", .cb = clFlush_callback, .ft = COMMON_FUNC_TYPE1},
    {.name = "clExtGetPreCompiledILOfFile",
     .cb = clExtGetPreCompiledILOfFile_callback,
     .ft = COMMON_FUNC_TYPE2}};
