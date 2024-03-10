#include <CL/cl.h>
#include <stdio.h>
#include <stdlib.h>

#define PLATFORM_COUNT 2
#define INTEL_PLATFORM_IDX 1
#define INTEL_DEVICE_COUNT 1
#define INTEL_DEVICE_IDX 0


typedef struct{
    cl_uchar* data;
    size_t size;
}  SpirvCode;

SpirvCode readSPIRVFromFile(const char *filename) {
    FILE *file = fopen(filename, "rb");
    SpirvCode ret = { NULL, 0 }; 

    if (!file) {
        printf("Couldn't open file '%s'!\n", filename);
        return ret; 
    }

    fseek(file, 0, SEEK_END);
    ret.size = ftell(file);
    rewind(file);

    ret.data = (cl_uchar*)malloc(ret.size);
    if (!ret.data) {
        printf("Memory allocation failed!\n");
        fclose(file);
        return ret; 
    }

    if (fread(ret.data, sizeof(cl_uchar), ret.size, file) != ret.size) {
        printf("Error reading file '%s'!\n", filename);
        free(ret.data);
        fclose(file);
        ret.data = NULL;
        ret.size = 0;
        return ret; 
    }

    fclose(file);
    return ret; 
}

int main()
{
    int err;                            // error code returned from api calls
      
    char paramBuffer[100000];
    unsigned int correct;               // number of correct results returned

    size_t gwx = 512;

    cl_uint numPlatforms;
    cl_platform_id platforms[PLATFORM_COUNT];
    cl_device_id devices[INTEL_DEVICE_COUNT];             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel

    

    ///////////////////////////////////////////////////////
    err = clGetPlatformIDs(PLATFORM_COUNT, platforms, NULL);
    if(err != CL_SUCCESS){
        printf("failed to get platform ids, err = %d\n", err);
    }
    else{
        clGetPlatformInfo(platforms[INTEL_PLATFORM_IDX],CL_PLATFORM_NAME,sizeof(paramBuffer),paramBuffer, NULL);
        printf("platform name: %s\n", paramBuffer);
    }

    ///////////////////////////////////////////////////////
    err = clGetDeviceIDs(platforms[INTEL_PLATFORM_IDX],CL_DEVICE_TYPE_GPU,1, devices, NULL);
    if(err != CL_SUCCESS){
        printf("failed to get device id, err = %d\n", err);
    }
    else{
        clGetDeviceInfo(devices[INTEL_DEVICE_IDX],CL_DEVICE_NAME,sizeof(paramBuffer),paramBuffer, NULL);
        printf("device name: %s\n", paramBuffer);
    }


    /////////////////////////////////////////////////////// 
    context = clCreateContext(NULL, 1, devices, NULL, NULL, &err);
    if(err != CL_SUCCESS){
        printf("failed to create context, err = %d\n", err);
    }
    else{
        printf("create context successful\n");
    }


    /////////////////////////////////////////////////////// 
    commands = clCreateCommandQueue(context, devices[INTEL_DEVICE_IDX], 0, &err);
    if(err != CL_SUCCESS){
        printf("failed to create command queue, err = %d\n", err);
    }
    else{
        printf("create command queue successful\n");
    }

    SpirvCode il = readSPIRVFromFile("C:/Users/30985/repo/ws/SimpleOpenCLSamples/samples/05_spirvkernelfromfile/sample_kernel64.spv");

    program = clCreateProgramWithIL(context, il.data, il.size, &err);
    if(err != CL_SUCCESS){
        printf("failed to create program with il, err = %d\n", err);
    }
    else{
        printf("create program successful\n");
    }

    /////////////////////////////////////////
    err = clBuildProgram(program, 1, devices, NULL,NULL,NULL);
    if(err != CL_SUCCESS){
        printf("failed to build program, err = %d\n", err);
    }
    else{
        printf("build program successful\n");
    }


    /////////////////////////////////////////
    kernel = clCreateKernel(program, "Test", &err);
    if(err != CL_SUCCESS){
        printf("failed to create kernel, err = %d\n", err);
    }
    else{
        printf("create kernel successful\n");
    }


    //////////////////////////////////////////
    cl_mem deviceMemDst = clCreateBuffer(
        context,
        CL_MEM_ALLOC_HOST_PTR,
        gwx * sizeof(cl_uint),
        NULL,
        NULL );

    // 设置内核参数
    clSetKernelArg(kernel, 0, sizeof(cl_mem), (void *)&deviceMemDst);

    // 执行内核
    size_t global_work_size[1] = {gwx};
    clEnqueueNDRangeKernel(
        commands,
        kernel,
        1,
        NULL,
        global_work_size,
        NULL,
        0,
        NULL,
        NULL );

    // 验证结果并打印前几个值
    if (gwx > 3) {
        cl_uint *ptr = (cl_uint *)clEnqueueMapBuffer(
            commands,
            deviceMemDst,
            CL_TRUE,
            CL_MAP_READ,
            0,
            gwx * sizeof(cl_uint),
            0,
            NULL,
            NULL,
            NULL );

        printf("First few values: [0] = %u, [1] = %u, [2] = %u\n", ptr[0], ptr[1], ptr[2]);

        clEnqueueUnmapMemObject(
            commands,
            deviceMemDst,
            (void *)ptr,
            0,
            NULL,
            NULL );
    }

    clFinish(commands);


    printf("Done\n");
}