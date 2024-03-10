//
// File:       hello.c
//
// Abstract:   A simple "Hello World" compute example showing basic usage of OpenCL which
//             calculates the mathematical square (X[i] = pow(X[i],2)) for a buffer of
//             floating point values.
//             
//
// Version:    <1.0>
//
// Disclaimer: IMPORTANT:  This Apple software is supplied to you by Apple Inc. ("Apple")
//             in consideration of your agreement to the following terms, and your use,
//             installation, modification or redistribution of this Apple software
//             constitutes acceptance of these terms.  If you do not agree with these
//             terms, please do not use, install, modify or redistribute this Apple
//             software.
//
//             In consideration of your agreement to abide by the following terms, and
//             subject to these terms, Apple grants you a personal, non - exclusive
//             license, under Apple's copyrights in this original Apple software ( the
//             "Apple Software" ), to use, reproduce, modify and redistribute the Apple
//             Software, with or without modifications, in source and / or binary forms;
//             provided that if you redistribute the Apple Software in its entirety and
//             without modifications, you must retain this notice and the following text
//             and disclaimers in all such redistributions of the Apple Software. Neither
//             the name, trademarks, service marks or logos of Apple Inc. may be used to
//             endorse or promote products derived from the Apple Software without specific
//             prior written permission from Apple.  Except as expressly stated in this
//             notice, no other rights or licenses, express or implied, are granted by
//             Apple herein, including but not limited to any patent rights that may be
//             infringed by your derivative works or by other works in which the Apple
//             Software may be incorporated.
//
//             The Apple Software is provided by Apple on an "AS IS" basis.  APPLE MAKES NO
//             WARRANTIES, EXPRESS OR IMPLIED, INCLUDING WITHOUT LIMITATION THE IMPLIED
//             WARRANTIES OF NON - INFRINGEMENT, MERCHANTABILITY AND FITNESS FOR A
//             PARTICULAR PURPOSE, REGARDING THE APPLE SOFTWARE OR ITS USE AND OPERATION
//             ALONE OR IN COMBINATION WITH YOUR PRODUCTS.
//
//             IN NO EVENT SHALL APPLE BE LIABLE FOR ANY SPECIAL, INDIRECT, INCIDENTAL OR
//             CONSEQUENTIAL DAMAGES ( INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
//             SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
//             INTERRUPTION ) ARISING IN ANY WAY OUT OF THE USE, REPRODUCTION, MODIFICATION
//             AND / OR DISTRIBUTION OF THE APPLE SOFTWARE, HOWEVER CAUSED AND WHETHER
//             UNDER THEORY OF CONTRACT, TORT ( INCLUDING NEGLIGENCE ), STRICT LIABILITY OR
//             OTHERWISE, EVEN IF APPLE HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
//
// Copyright ( C ) 2008 Apple Inc. All Rights Reserved.
//

////////////////////////////////////////////////////////////////////////////////

#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <CL/cl.h>

////////////////////////////////////////////////////////////////////////////////

// Use a static data size for simplicity
//
#define DATA_SIZE (1024)

////////////////////////////////////////////////////////////////////////////////

// Simple compute kernel which computes the square of an input array 
//
const char *KernelSource = "\n" \
"__kernel void square(                                                       \n" \
"   __global float* input,                                              \n" \
"   __global float* output,                                             \n" \
"   const unsigned int count)                                           \n" \
"{                                                                      \n" \
"   int i = get_global_id(0);                                           \n" \
"   if(i < count)                                                       \n" \
"       output[i] = input[i] * input[i];                                \n" \
"}                                                                      \n" \
"\n";

////////////////////////////////////////////////////////////////////////////////
int platformSupportsSPIRV(cl_platform_id platform) {
    cl_int ret;
    char extensions[102400];

    // 获取平台的扩展字符串
    ret = clGetPlatformInfo(platform, CL_PLATFORM_EXTENSIONS, sizeof(extensions), extensions, NULL);
    if (ret != CL_SUCCESS) {
        printf("Error getting platform extensions, err ret = %d\n",ret);
        return 0; // 返回0表示出错或不支持SPIR-V
    }

    // 检查扩展字符串中是否包含cl_khr_il_program
    if (strstr(extensions, "cl_khr_il_program") != NULL) {
        return 1; // 返回1表示支持SPIR-V
    } else {
        return 0; // 返回0表示不支持SPIR-V
    }
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   fread(program_buffer, sizeof(char), program_size, program_handle);
   fclose(program_handle);

   /* Create program from file 

   Creates a program from the source code in the add_numbers.cl file. 
   Specifically, the code reads the file's content into a char array 
   called program_buffer, and then calls clCreateProgramWithSource.
   */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program 

   The fourth parameter accepts options that configure the compilation. 
   These are similar to the flags used by gcc. For example, you can 
   define a macro with the option -DMACRO=VALUE and turn off optimization 
   with -cl-opt-disable.
   */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}


cl_device_id getFirstSPIRVDevice(cl_platform_id *platforms, cl_uint numPlatforms) {
    cl_device_id device = NULL;

    for (cl_uint i = 0; i < numPlatforms; ++i) {
        if(!platformSupportsSPIRV(platforms[i]))
            continue;

        char pv[1024];
        cl_uint er = clGetPlatformInfo(platforms[i],CL_PLATFORM_VENDOR,sizeof(pv),pv,NULL);
        
        if (er != CL_SUCCESS) {
            printf("Error get platform %s\n", er);
        }else{
            printf("platform: %s\n", pv);
        }
        cl_uint numDevices;
        cl_int ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &numDevices);
        if (ret != CL_SUCCESS) {
            printf("Error getting device count for platform %d\n", i);
            continue;
        }

        cl_device_id *devices = (cl_device_id*)malloc(sizeof(cl_device_id) * numDevices);
        ret = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numDevices, devices, NULL);
        if (ret != CL_SUCCESS) {
            printf("Error getting device IDs for platform %d\n", i);
            free(devices);
            continue;
        }

        for (cl_uint j = 0; j < numDevices; ++j) {
            cl_bool supportsSPIRV;
            ret = clGetDeviceInfo(devices[j], 0x105B, 0, NULL, NULL);
            if (ret == CL_SUCCESS) {
                supportsSPIRV = CL_TRUE;
            } else {
                supportsSPIRV = CL_FALSE;
            }

            if (supportsSPIRV) {
                printf("get intel platform device %d\n",j);
                device = devices[j];
                free(devices);
                return device;
            }
        }

        free(devices);
    }

    return device;
}


typedef struct {
    cl_uchar *data;
    size_t size;
} ByteData;

ByteData readSPIRVFromFile(const char *filename) {
    FILE *file = fopen(filename, "rb"); // 以二进制模式打开文件
    ByteData ret = { NULL, 0 }; // 初始化返回的字节数据

    if (!file) {
        printf("Couldn't open file '%s'!\n", filename);
        return ret; // 返回空字节数据
    }

    // 获取文件大小
    fseek(file, 0, SEEK_END);
    ret.size = ftell(file);
    rewind(file);

    // 分配内存以存储文件数据
    ret.data = (cl_uchar*)malloc(ret.size);
    if (!ret.data) {
        printf("Memory allocation failed!\n");
        fclose(file);
        return ret; // 返回空字节数据
    }

    // 从文件中读取数据到内存中
    if (fread(ret.data, sizeof(cl_uchar), ret.size, file) != ret.size) {
        printf("Error reading file '%s'!\n", filename);
        free(ret.data);
        fclose(file);
        ret.data = NULL;
        ret.size = 0;
        return ret; // 返回空字节数据
    }

    fclose(file);
    return ret; // 返回读取到的字节数据
}


int main()
{
    int err;                            // error code returned from api calls
      
    float data[DATA_SIZE];              // original data set given to device
    float results[DATA_SIZE];           // results returned from device
    unsigned int correct;               // number of correct results returned

    size_t global;                      // global domain size for our calculation
    size_t local;                       // local domain size for our calculation

    cl_uint numPlatforms;
    cl_platform_id* platforms;
    cl_device_id device_id;             // compute device id 
    cl_context context;                 // compute context
    cl_command_queue commands;          // compute command queue
    cl_program program;                 // compute program
    cl_kernel kernel;                   // compute kernel
    
    cl_mem input;                       // device memory used for the input array
    cl_mem output;                      // device memory used for the output array
    
    // Fill our data set with random float values
    //
    int i = 0;
    unsigned int count = DATA_SIZE;
    for(i = 0; i < count; i++)
        data[i] = rand() / (float)RAND_MAX;
    
    // Connect to a compute device
    //

    err = clGetPlatformIDs(0, NULL, &numPlatforms);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to get numPlatforms!, err = %d\n",err);
        return EXIT_FAILURE;
    } 

    platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numPlatforms);
    err = clGetPlatformIDs(numPlatforms, platforms, NULL);
    if(err != CL_SUCCESS) {
        printf("Error: Failed to get platforms!, err = %d\n",err);
        return EXIT_FAILURE;
    } 


    device_id = getFirstSPIRVDevice(platforms,numPlatforms);
    char paramValue[1024];
    cl_uint ret = clGetDeviceInfo(device_id, CL_DEVICE_VERSION, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Device Version: %s\n", paramValue);
    }

    // 获取驱动版本
    ret = clGetDeviceInfo(device_id, CL_DEVICE_NAME, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Device Name: %s\n", paramValue);
    }

    // Create a compute context 
    //
    context = clCreateContext(0, 1, &device_id, NULL, NULL, &err);
    if(err != CL_SUCCESS){
        printf("Error: Failed to create context!\n");
    }
    if (!context)
    {
        printf("Error: Failed to create a compute context!\n");
        return EXIT_FAILURE;
    }

    // Create a command commands
    //
    commands = clCreateCommandQueue(context, device_id, 0, &err);
    if (!commands)
    {
        printf("Error: Failed to create a command commands!\n");
        return EXIT_FAILURE;
    }

    // Create the compute program from the source buffer
    //
    
    // ByteData il = readSPIRVFromFile("C:/Users/30985/repo/ws/opencl2wasm/examples/square.spv");

    // printf("read spirv , il.size = %d\n", il.size);
    // // printf("li data:");
    // //     for(size_t i=0; i < il.size; ++i){
    // //         if(i % 20 == 0)printf("\n");
    // //         printf("%02x", il.data[i]);
    // //     }

    // program = clCreateProgramWithIL(context, il.data, il.size, &err);
    program = build_program(context, device_id, "C:/Users/30985/repo/ws/opencl2wasm/examples/square.cl");

    printf("here\n");
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to create program with IL, err = %d!\n",err);
        exit(1);
    }

    // program = clCreateProgramWithSource(context, 1, (const char **) & KernelSource, NULL, &err);
    if (!program)
    {
        printf("Error: Failed to create compute program!\n");
        return EXIT_FAILURE;
    }

    // Build the program executable
    //
    // printf("before get program info.\n");
    // cl_uint numd;
    // err = clGetProgramInfo(program, CL_PROGRAM_NUM_DEVICES, sizeof(numd), &numd, NULL);
    // if(err != CL_SUCCESS){
    //     printf("cl get program info failed.\n");
    // }else{
    //     printf("cl get program info: %s\n", paramValue);
    // }
    
    printf("before build program\n");
    err = clBuildProgram(program, 1, &device_id, NULL, NULL, NULL);
    
    printf("here 2\n");
    if (err != CL_SUCCESS)
    {
        size_t len;
        char buffer[2048];

        printf("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(program, device_id, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
        printf("%s\n", buffer);
        exit(1);
    }

    // Create the compute kernel in the program we wish to run
    //
    kernel = clCreateKernel(program, "square", &err);
    if (!kernel || err != CL_SUCCESS)
    {
        printf("Error: Failed to create compute kernel!\n");
        exit(1);
    }

    // Create the input and output arrays in device memory for our calculation
    //
    input = clCreateBuffer(context,  CL_MEM_READ_ONLY,  sizeof(float) * count, NULL, NULL);
    output = clCreateBuffer(context, CL_MEM_WRITE_ONLY, sizeof(float) * count, NULL, NULL);
    if (!input || !output)
    {
        printf("Error: Failed to allocate device memory!\n");
        exit(1);
    }    
    
    // Write our data set into the input array in device memory 
    //
    err = clEnqueueWriteBuffer(commands, input, CL_TRUE, 0, sizeof(float) * count, data, 0, NULL, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to write to source array!\n");
        exit(1);
    }

    // Set the arguments to our compute kernel
    //
    err = 0;
    err  = clSetKernelArg(kernel, 0, sizeof(cl_mem), &input);
    err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &output);
    err |= clSetKernelArg(kernel, 2, sizeof(unsigned int), &count);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to set kernel arguments! %d\n", err);
        exit(1);
    }

    // Get the maximum work group size for executing the kernel on the device
    //
    err = clGetKernelWorkGroupInfo(kernel, device_id, CL_KERNEL_WORK_GROUP_SIZE, sizeof(local), &local, NULL);
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to retrieve kernel work group info! %d\n", err);
        exit(1);
    }

    // Execute the kernel over the entire range of our 1d input data set
    // using the maximum number of work group items for this device
    //
    global = count;
    err = clEnqueueNDRangeKernel(commands, kernel, 1, NULL, &global, &local, 0, NULL, NULL);
    if (err)
    {
        printf("Error: Failed to execute kernel!\n");
        return EXIT_FAILURE;
    }

    // Wait for the command commands to get serviced before reading back results
    //
    clFinish(commands);

    // Read back the results from the device to verify the output
    //
    err = clEnqueueReadBuffer( commands, output, CL_TRUE, 0, sizeof(float) * count, results, 0, NULL, NULL );  
    if (err != CL_SUCCESS)
    {
        printf("Error: Failed to read output array! %d\n", err);
        exit(1);
    }
    
    // Validate our results
    //
    correct = 0;
    for(i = 0; i < count; i++)
    {
        if(results[i] == data[i] * data[i])
            correct++;
    }
    
    // Print a brief summary detailing the results
    //
    printf("Computed '%d/%d' correct values!\n", correct, count);
    
    // Shutdown and cleanup
    //
    free(platforms);
    clReleaseMemObject(input);
    clReleaseMemObject(output);
    clReleaseProgram(program);
    clReleaseKernel(kernel);
    clReleaseCommandQueue(commands);
    clReleaseContext(context);

    return 0;
}