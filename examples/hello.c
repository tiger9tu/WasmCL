// C standard includes
#include <stdio.h>
#include <stdlib.h>
#include <CL/cl.h>
#include <string.h>
void printDeviceInfo(cl_device_id device) {
    cl_int ret;
    char paramValue[1024];

    // 获取设备名称
    ret = clGetDeviceInfo(device, CL_DEVICE_NAME, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Device Name: %s\n", paramValue);
    }

    // 获取设备供应商
    ret = clGetDeviceInfo(device, CL_DEVICE_VENDOR, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Device Vendor: %s\n", paramValue);
    }

    // 获取设备供应商ID
    ret = clGetDeviceInfo(device, CL_DEVICE_VENDOR_ID, sizeof(cl_uint), &paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Device Vendor ID: %u\n", *((cl_uint*)paramValue));
    }

    // 获取设备类型
    // ret = clGetDeviceInfo(device, CL_DEVICE_TYPE, sizeof(cl_device_type), &paramValue, NULL);
    // if (ret == CL_SUCCESS) {
    //     printf("Device Type: ");
    //     if (paramValue & CL_DEVICE_TYPE_CPU) printf("CPU ");
    //     if (paramValue & CL_DEVICE_TYPE_GPU) printf("GPU ");
    //     if (paramValue & CL_DEVICE_TYPE_ACCELERATOR) printf("Accelerator ");
    //     if (paramValue & CL_DEVICE_TYPE_DEFAULT) printf("Default ");
    //     printf("\n");
    // }

    // 获取设备版本
    ret = clGetDeviceInfo(device, CL_DEVICE_VERSION, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Device Version: %s\n", paramValue);
    }

    // 获取驱动版本
    ret = clGetDeviceInfo(device, CL_DRIVER_VERSION, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("Driver Version: %s\n", paramValue);
    }

    // 获取设备支持的OpenCL C版本
    ret = clGetDeviceInfo(device, 0x103D, sizeof(paramValue), paramValue, NULL);
    if (ret == CL_SUCCESS) {
        printf("OpenCL C Version: %s\n", paramValue);
    }

    // 获取设备全局内存大小
    cl_ulong globalMemSize;
    ret = clGetDeviceInfo(device, CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &globalMemSize, NULL);
    if (ret == CL_SUCCESS) {
        printf("Global Memory Size: %llu bytes\n", globalMemSize);
    }

    // 获取设备本地内存大小
    cl_ulong localMemSize;
    ret = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &localMemSize, NULL);
    if (ret == CL_SUCCESS) {
        printf("Local Memory Size: %llu bytes\n", localMemSize);
    }

    // 获取设备最大工作组大小
    size_t maxWorkGroupSize;
    ret = clGetDeviceInfo(device, CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(size_t), &maxWorkGroupSize, NULL);
    if (ret == CL_SUCCESS) {
        printf("Max Work Group Size: %zu\n", maxWorkGroupSize);
    }

    // 其他需要的信息可以通过类似的方式获取和打印

    printf("\n");
}

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

cl_device_id getFirstSPIRVDevice(cl_platform_id *platforms, cl_uint numPlatforms) {
    cl_device_id device = NULL;

    for (cl_uint i = 0; i < numPlatforms; ++i) {
        if(!platformSupportsSPIRV(platforms[i]))
            continue;

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
                device = devices[j];
                free(devices);
                return device;
            }
        }

        free(devices);
    }

    return device;
}

int main() {
    cl_uint numPlatforms;
    clGetPlatformIDs(0, NULL, &numPlatforms);

    cl_platform_id *platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numPlatforms);
    clGetPlatformIDs(numPlatforms, platforms, NULL);

    cl_device_id device = getFirstSPIRVDevice(platforms, numPlatforms);

    if (device != NULL) {
        printf("First SPIR-V supporting device found.\n");
        printDeviceInfo(device);
        // 可以在这里进行使用该设备的操作
    } else {
        printf("No SPIR-V supporting device found.\n");
    }

    free(platforms);
    return 0;
}
