#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PLATFORM_COUNT 2
#define INTEL_PLATFORM_IDX 1
#define INTEL_DEVICE_COUNT 1
#define INTEL_DEVICE_IDX 0
#define MAX_SPV_LEN 1000
#define SPIRV_FILE "sample_kernel64.spv"

extern cl_int clExtGetPreCompiledILOfFile(cl_uchar *data_buf, size_t *len);

typedef struct {
  cl_uchar data[MAX_SPV_LEN];
  size_t size;
} SpirvCode;

int main() {
  int err; // error code returned from api calls

  char paramBuffer[1000];
  unsigned int correct; // number of correct results returned

  size_t gwx = 512;

  cl_uint numPlatforms;
  cl_platform_id platforms[PLATFORM_COUNT];
  cl_device_id devices[INTEL_DEVICE_COUNT]; // compute device id
  cl_context context;                       // compute context
  cl_command_queue commands;                // compute command queue
  cl_program program;                       // compute program
  cl_kernel kernel;                         // compute kernel

  ///////////////////////////////////////////////////////
  err = clGetPlatformIDs(PLATFORM_COUNT, platforms, NULL);
  if (err != CL_SUCCESS) {
    printf("failed to get platform ids, err = %d\n", err);
  } else {
    clGetPlatformInfo(platforms[INTEL_PLATFORM_IDX], CL_PLATFORM_NAME,
                      sizeof(paramBuffer), paramBuffer, NULL);
    printf("platform name: %s\n", paramBuffer);
  }

  // ///////////////////////////////////////////////////////
  err = clGetDeviceIDs(platforms[INTEL_PLATFORM_IDX], CL_DEVICE_TYPE_GPU, 1,
                       devices, NULL);
  if (err != CL_SUCCESS) {
    printf("failed to get device id, err = %d\n", err);
  } else {
    clGetDeviceInfo(devices[INTEL_DEVICE_IDX], CL_DEVICE_NAME,
                    sizeof(paramBuffer), paramBuffer, NULL);
    printf("device name: %s\n", paramBuffer);
  }

  ///////////////////////////////////////////////////////
  context = clCreateContext(NULL, 1, devices, NULL, NULL, &err);
  if (err != CL_SUCCESS) {
    printf("failed to create context, err = %d\n", err);
  } else {
    printf("create context successful\n");
  }

  ///////////////////////////////////////////////////////
  commands = clCreateCommandQueue(context, devices[INTEL_DEVICE_IDX], 0, &err);
  if (err != CL_SUCCESS) {
    printf("failed to create command queue, err = %d\n", err);
  } else {
    printf("create command queue successful\n");
  }

  // SpirvCode il = readSPIRVFromFile(SPIRV_FILE);
  SpirvCode il;
  err = clExtGetPreCompiledILOfFile(il.data, &il.size);

  program = clCreateProgramWithIL(context, il.data, il.size, &err);
  if (err != CL_SUCCESS) {
    printf("failed to create program with il, err = %d\n", err);
  } else {
    printf("create program successful\n");
  }

  /////////////////////////////////////////
  err = clBuildProgram(program, 1, devices, NULL, NULL, NULL);
  if (err != CL_SUCCESS) {
    printf("failed to build program, err = %d\n", err);
  } else {
    printf("build program successful\n");
  }

  /////////////////////////////////////////
  kernel = clCreateKernel(program, "Test", &err);
  if (err != CL_SUCCESS) {
    printf("failed to create kernel, err = %d\n", err);
  } else {
    printf("create kernel successful\n");
  }

  //////////////////////////////////////////
  cl_mem deviceMemDst = clCreateBuffer(context, CL_MEM_ALLOC_HOST_PTR,
                                       gwx * sizeof(cl_uint), NULL, NULL);

  // 设置内核参数
  clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&deviceMemDst);

  // 执行内核
  size_t global_work_size[1] = {gwx};
  clEnqueueNDRangeKernel(commands, kernel, 1, NULL, global_work_size, NULL, 0,
                         NULL, NULL);

  printf("after clenqueueNDRangeKernel\n");
  // 验证结果并打印前几个值

  if (gwx > 3) {
    int res[3];
    err = clEnqueueReadBuffer(commands, deviceMemDst, CL_TRUE, 0, sizeof(res),
                              res, 0, NULL, NULL); // <=====GET OUTPUT

    printf("First few values: [0] = %u, [1] = %u, [2] = %u\n", res[0], res[1],
           res[2]);
  }

  clFinish(commands);

  printf("Done\n");
}