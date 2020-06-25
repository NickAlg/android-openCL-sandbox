#include "CLbackend.h"
#include "android_log.h"

#define DEBUG_PRINT 1
#define DEBUG_TIME 0

#define KERNEL(...)#__VA_ARGS__

const char *kernelSourceCode_grad_smootch = KERNEL(

        __kernel
        void grad_smootch(__global uchar
        *src_buffer,
                __global
        uchar *grad_buffer,
                __global
        uchar *dst_buffer,
        const int thresh,
        const int radius)
{
        //const int gidx = get_global_id(0);
        //const int gidy = get_global_id(1);
        int work_width = get_global_size(0);
        int work_height = get_global_size(1);
        int bokx = get_global_id(0);
        int boky = get_global_id(1);
        //size_t lidx = get_local_id(0);
        int dst_pixel_id = boky * work_width + bokx;
        int grad_data = (int)grad_buffer[dst_pixel_id];

        if (grad_data < thresh )
{
        int sumWeight = 0;
        int sumC0 = 0;

        int compute_up_range = min(boky, radius);
        int compute_down_range = min(work_height - 1 - boky, radius);
        int compute_left_range = min(bokx, radius);
        int compute_right_range = min(work_width - 1 - bokx, radius);
        for (int kr = -compute_up_range; kr <= compute_down_range; kr+=2)
{
        int offset = (boky + kr) * work_width + bokx;
        for (int kc = -compute_left_range; kc <= compute_right_range; kc+=2)
{
        if ( abs(src_buffer[dst_pixel_id] - src_buffer[offset + kc]) < 10 )
{
        sumC0 +=  (int)src_buffer[offset + kc];
        sumWeight++;
}
}
}
        if (sumWeight != 0)
{
        dst_buffer[dst_pixel_id] = (uchar)(sumC0 / sumWeight);
}

}
        else
{
        dst_buffer[dst_pixel_id] = src_buffer[dst_pixel_id];
}

}

);

CLRuntime::CLRuntime() {

}

cl_context CLRuntime::get_context() const {
    return m_context;
}

cl_command_queue CLRuntime::get_command_queue() const {
    return m_cmd_queue;
}

bool CLRuntime::IsInited() const {
    return m_inited_;
}

cl_program CLRuntime::make_program(const char **program_source,
                                   cl_uint program_source_len) {
    cl_int err = 0;
    cl_program program =
            clCreateProgramWithSource(m_context, program_source_len, program_source, NULL, &err);
    if (err != CL_SUCCESS) {
        LOG_W2(ANDROID_LOG_ERROR, "  with clCreateProgramWithSource. << %d \n",
               err);
    }
//https://www.khronos.org/registry/OpenCL/sdk/1.0/docs/man/xhtml/clBuildProgram.html cl_khr_fp16
    err = clBuildProgram(program,
                         0,
                         NULL,
                         "-cl-mad-enable -cl-fast-relaxed-math -cl-single-precision-constant",
                         NULL,
                         NULL);
    if (err != CL_SUCCESS) {
        LOG_W2(ANDROID_LOG_ERROR, " with clBuildProgram %d \n", err);

        size_t len = 0;
        err = clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
        char log[len];
        log[0] = 0;
        err = clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, len, log, NULL);
        if (err == CL_INVALID_VALUE) {
            LOG_W2(ANDROID_LOG_ERROR,
                   " There was a build error, but there is insufficient space allocated to show the build logs. \n");
        } else {
            LOG_W2(ANDROID_LOG_ERROR, " There was a build error, Build error:. %s \n", log);

        }
    }

    m_programs.push_back(program);

    return program;
}

cl_kernel CLRuntime::make_kernel(const std::string &kernel_name,
                                 cl_program program) {
    cl_int err;
    cl_kernel kernel = clCreateKernel(program, kernel_name.c_str(), &err);
    if (err != CL_SUCCESS) {
        LOG_W2(ANDROID_LOG_ERROR, " with %s clCreateKernel. %d \n",
               kernel_name.c_str(),
               err);
        if (err == CL_INVALID_PROGRAM_EXECUTABLE) {
            size_t len = 0;
            err = clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
            char buffer[len];
            err = clGetProgramBuildInfo(program, m_device, CL_PROGRAM_BUILD_LOG, len, buffer, NULL);
            LOG_W2(ANDROID_LOG_ERROR,
                   " with clCreateKernel. %s CL_INVALID_PROGRAM_EXECUTABLE> %s \n",
                   kernel_name.c_str(),
                   buffer);
        }
    }
    m_kernels.push_back(kernel);
    return kernel;
}

cl_command_queue CLRuntime::make_queue(cl_int *err) {
    cl_command_queue q = clCreateCommandQueue(m_context, m_device, 0, err);
    return q;
}

std::string CLRuntime::GetDevTypeSTR() const {

    if (m_dev_type_ == CL_DEVICE_TYPE_CPU) {
        return std::string("CPU");
    } else if (m_dev_type_ == CL_DEVICE_TYPE_GPU) {
        return std::string("GPU");
    } else if (m_dev_type_ == CL_DEVICE_TYPE_ACCELERATOR) {
        return std::string("ACCELERATOR");
    } else if (m_dev_type_ == CL_DEVICE_TYPE_DEFAULT) {
        return std::string("DEFAULT");
    }
    return std::string("UNCOOL");;
}

std::string CLRuntime::ObtainCLDeviceName(cl_device_id device) const {
    std::string tmp1;
    char *value;
    size_t valueSize;
    clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &valueSize);
    value = (char *) malloc(valueSize);
    clGetDeviceInfo(device, CL_DEVICE_NAME, valueSize, value, NULL);
    tmp1.append(std::string(value));
    free(value);
    return tmp1;
}

std::string
CLRuntime::getInfo() {
    cl_int status;//print some info
    std::string
            tmp1;
    char buffer[256];
    //Query the name of the device
    m_device_name = ObtainCLDeviceName(get_deviceID());
    LOG_W2(ANDROID_LOG_WARN, " Device Name:%s \n", m_device_name.c_str());
    tmp1.append(m_device_name);

    //Query device type
    cl_device_type dev_type;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_TYPE, sizeof(cl_device_type), &dev_type,
                             NULL);
    snprintf(buffer, sizeof(buffer), "Device type: %lu ", dev_type);
    tmp1 += std::string
            (buffer);

    if (dev_type == CL_DEVICE_TYPE_CPU) {
        tmp1.append("CL_DEVICE_TYPE_CPU \n");
    } else if (dev_type == CL_DEVICE_TYPE_GPU) {
        tmp1.append("CL_DEVICE_TYPE_GPU\n");
    } else if (dev_type == CL_DEVICE_TYPE_ACCELERATOR) {
        tmp1.append("CL_DEVICE_TYPE_ACCELERATOR\n");
    } else if (dev_type == CL_DEVICE_TYPE_DEFAULT) {
        tmp1.append("CL_DEVICE_TYPE_DEFAULT\n");
    }

    //Query the maximum number of computing units of the device
    cl_uint UnitNum;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &UnitNum,
                             NULL);

    snprintf(buffer, sizeof(buffer), "Compute Units Number: %d\n", UnitNum);
    tmp1.append(std::string
                        (buffer));

    //Query the maximum number of sub devices
    cl_uint maxPartitionSubDevices;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_PARTITION_MAX_SUB_DEVICES, sizeof(cl_uint),
                             &maxPartitionSubDevices, NULL);
    snprintf(buffer, sizeof(buffer), "Device Partition Max Sub Devices: %d\n", UnitNum);
    tmp1.append(std::string
                        (buffer));

    //Query device core frequency
    cl_uint frequency;
    status =
            clGetDeviceInfo(get_deviceID(), CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint),
                            &frequency,
                            NULL);
    snprintf(buffer, sizeof(buffer), "Device Frequency: %d(MHz)\n", frequency);
    tmp1.append(std::string
                        (buffer));
    // Query device global memory size
    cl_ulong GlobalSize;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong),
                             &GlobalSize,
                             NULL);
    snprintf(buffer, sizeof(buffer), "Device Global Mem Size: %0.0f(MB)\n",
             (float) GlobalSize / 1024 / 1024);

    tmp1.append(std::string
                        (buffer));
    //Query device global memory cache line
    cl_uint GlobalCacheLine;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint),
                             &GlobalCacheLine, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global Mem CacheLine: %d(Byte)\n", GlobalCacheLine);
    tmp1.append(std::string
                        (buffer));
    //Query the largest working group of equipment
    cl_ulong GlobalWGSize;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong),
                             &GlobalWGSize, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global MAX_WORK_GROUP Size: %lu\n", GlobalWGSize);
    tmp1.append(std::string
                        (buffer));
    //Query the largest working group of equipment
    cl_ulong GlobalWISize;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(cl_ulong),
                             &GlobalWISize, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global MAX_WORK_ITEM Size: %lu\n", GlobalWISize);
    tmp1.append(std::string
                        (buffer));

    cl_ulong LOCAL_MEM_SIZE;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong),
                             &LOCAL_MEM_SIZE, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global LOCAL_MEM_SIZE Size: %0.0f(KB)\n",
             (float) LOCAL_MEM_SIZE / 1024);
    tmp1.append(std::string
                        (buffer));

    //Query the OpenCL version supported by the device
    char DeviceVersion[210];
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_VERSION, 100, DeviceVersion, NULL);
    snprintf(buffer, sizeof(buffer), "Device Version:%s\n", DeviceVersion);
    tmp1.append(std::string
                        (buffer));
    //Query device extension
    char *DeviceExtensions;
    size_t ExtenNum;
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_EXTENSIONS, 0, NULL, &ExtenNum);
    DeviceExtensions = (char *) malloc(ExtenNum);
    status = clGetDeviceInfo(get_deviceID(), CL_DEVICE_EXTENSIONS, ExtenNum, DeviceExtensions,
                             NULL);
    snprintf(buffer, sizeof(buffer), "Device Extensions: %s\n", DeviceExtensions);
    tmp1.append(std::string
                        (buffer));

    LOG_W2(ANDROID_LOG_WARN, "Check  %s", tmp1.c_str());
    return tmp1;
}

std::stringstream CLRuntime::getStringstream(char *info,
                                             cl_uint nPlatform,
                                             cl_platform_id *listPlatform,
                                             cl_uint &nDevice,
                                             cl_device_id *listDevice) {
    std::stringstream ret_inf;
    for (int i = 0; i < nPlatform; i++) {
        clGetPlatformInfo(listPlatform[i], CL_PLATFORM_NAME, 1024, info, NULL);
        LOGD("Platfom[%d]:\n\tName\t\t%s", i, info);
        ret_inf << info;

        clGetPlatformInfo(listPlatform[i], CL_PLATFORM_VERSION, 1024, info, NULL);
        LOGD("\n\tVersion\t\t%s", info);
        clGetPlatformInfo(listPlatform[i], CL_PLATFORM_VENDOR, 1024, info, NULL);
        LOGD("\n\tVendor\t\t%s", info);
        clGetPlatformInfo(listPlatform[i], CL_PLATFORM_PROFILE, 1024, info, NULL);
        LOGD("\n\tProfile\t\t%s", info);
        clGetPlatformInfo(listPlatform[i], CL_PLATFORM_EXTENSIONS, 1024, info, NULL);
        LOGD("\n\tExtension\t%s", info);
        ret_inf << info;

        clGetDeviceIDs(listPlatform[i], CL_DEVICE_TYPE_ALL, 0, NULL, &nDevice);
        listDevice = (cl_device_id *) malloc(nDevice * sizeof(cl_device_id));
        clGetDeviceIDs(listPlatform[i], CL_DEVICE_TYPE_ALL, nDevice, listDevice, NULL);

        for (int j = 0; j < nDevice; j++) {
//      LOGD("\n");
            clGetDeviceInfo(listDevice[j], CL_DEVICE_NAME, 1024, info, NULL);
            LOGD("\n\tDevice[%d]:\n\tName\t\t%s", j, info);
            ret_inf << info;
            clGetDeviceInfo(listDevice[j], CL_DEVICE_VERSION, 1024, info, NULL);
            LOGD("\n\tVersion\t\t%s", info);
            ret_inf << info;
            clGetDeviceInfo(listDevice[j], CL_DEVICE_TYPE, 1024, info, NULL);
            switch (info[0]) {
                case CL_DEVICE_TYPE_DEFAULT:
                    strcpy(info, "DEFAULT");
                    break;
                case CL_DEVICE_TYPE_CPU:
                    strcpy(info, "CPU");
                    break;
                case CL_DEVICE_TYPE_GPU:
                    strcpy(info, "GPU");
                    break;
                case CL_DEVICE_TYPE_ACCELERATOR:
                    strcpy(info, "ACCELERATOR");
                    break;
                case CL_DEVICE_TYPE_CUSTOM:
                    strcpy(info, "CUSTOM");
                    break;
                case CL_DEVICE_TYPE_ALL:
                    strcpy(info, "ALL");
                    break;
            }
            LOGD("\n\tType\t\t%s", info);
            ret_inf << info;

            cl_device_svm_capabilities svm;
            clGetDeviceInfo(listDevice[j],
                            CL_DEVICE_VERSION,
                            sizeof(cl_device_svm_capabilities),
                            &svm,
                            NULL);
            info[0] = '\0';
            if (svm & CL_DEVICE_SVM_COARSE_GRAIN_BUFFER) {
                strcat(info, "COARSE_GRAIN_BUFFER ");
            }
            if (svm & CL_DEVICE_SVM_FINE_GRAIN_BUFFER) {
                strcat(info, "FINE_GRAIN_BUFFER ");
            }
            if (svm & CL_DEVICE_SVM_FINE_GRAIN_SYSTEM) {
                strcat(info, "FINE_GRAIN_SYSTEM ");
            }
            if (svm & CL_DEVICE_SVM_ATOMICS) {
                strcat(info, "ATOMICS");
            }
            LOGD("\n\tSVM\t\t%s", info);
            ret_inf << info;
        }
        printf("\n\n");
        free(listDevice);
    }
    return ret_inf;
}

int CLRuntime::init() {
    long t1, t2;
    char info[1024];

    cl_int status = CL_SUCCESS;
    NNM::OpenCLSymbolsOperator::createOpenCLSymbolsOperatorSingleInstance();
    if (nullptr == NNM::OpenCLSymbolsOperator::getOpenclSymbolsPtr()) {
        LOGD("OpenCL init error , callback ...");
        return EXIT_FAILURE;
    } else {
        LOGD("OpenCL init ok");
    }

    // Get opencl available platforms, currently ARM
    //auto ret_inf = getStringstream( info, nPlatform, listPlatforms, nDevice, listDevice);

    status = clGetPlatformIDs(0, NULL, &nPlatform);
    if (status != CL_SUCCESS) {
        LOGE("Error: Getting Platforms\n");
        return EXIT_FAILURE;
    }

    if (nPlatform > 0) {
        cl_platform_id *platforms = (cl_platform_id *) malloc(
                nPlatform * sizeof(cl_platform_id));
        status = clGetPlatformIDs(nPlatform, platforms, NULL);
        if (status != CL_SUCCESS) {
            LOGE(
                    "Error: Getting Platform Ids.(clGetPlatformIDs)\n");
            return EXIT_FAILURE;
        }

        listPlatform = platforms[0];
        free(platforms);
    }

    // If we can find the corresponding platform, use it, otherwise return NULL
    cl_context_properties cps[3] = {
            CL_CONTEXT_PLATFORM,
            (cl_context_properties) listPlatform,
            0
    };

    cl_context_properties *cprops = (NULL == listPlatform) ? NULL : cps;


    //Generate context
    gpu_context = clCreateContextFromType(
            cprops,
            CL_DEVICE_TYPE_GPU,
            NULL,
            NULL,
            &status);
    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Creating Context.(clCreateContexFromType)\n");
        return EXIT_FAILURE;
    }

// Looking for OpenCL equipment

    // First get the length of the device list
    status = clGetContextInfo(gpu_context,
                              CL_CONTEXT_DEVICES,
                              0,
                              NULL,
                              &deviceListSize);
    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Getting Context Info device list size, clGetContextInfo)\n");
        return EXIT_FAILURE;
    }

    listDevice = (cl_device_id *) malloc(sizeof(cl_device_id) * deviceListSize);
    if (listDevice == NULL) {
        LOGE("Error: No listDevice found.\n");
        return EXIT_FAILURE;
    }

    // Now get the device list
    status = clGetContextInfo(gpu_context,
                              CL_CONTEXT_DEVICES,
                              deviceListSize,
                              listDevice,
                              NULL);
    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Getting Context Info (device list, clGetContextInfo)\n");
        return EXIT_FAILURE;
    }

    if (iniKernelsgrandsmoothTest(kernel_gradsmooth) != CL_SUCCESS) {
        LOGE(
                "Error: Creating kernel_gradsmooth from program.(clCreateKernel)\n");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

int CLRuntime::iniKernelsgrandsmoothTest(cl_kernel &kernel_gradsmooth) const {
    // Load kernel program, compile CL program, generate CL kernel instance
    cl_int status = CL_SUCCESS;

    size_t sourceSize_gradsmooth[] = {strlen(kernelSourceCode_grad_smootch)};
    cl_program program_gradsmooth = clCreateProgramWithSource(gpu_context,
                                                              1,
                                                              &kernelSourceCode_grad_smootch,
                                                              sourceSize_gradsmooth,
                                                              &status);

    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Loading Binary into cl_program (clCreateProgramWithBinary)\n");
        return EXIT_FAILURE;
    }

// Compile CL program for the specified device.
    const char options[] = "-cl-single-precision-constant -cl-fast-relaxed-math";

//    LOGE( "***iniKernelsgrandsmoothTest 3  ***\n");

    //gradsmooth
    status = clBuildProgram(program_gradsmooth, 1, listDevice, options, NULL, NULL);
    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Building program_gradsmooth (clBuildingProgram) err_code:%d \n",
                status);
        return EXIT_FAILURE;
    }

    // Get the handle of the kernel instance with the specified name
    kernel_gradsmooth = clCreateKernel(program_gradsmooth, "grad_smootch", &status);
    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Creating kernel_gradsmooth from program.(clCreateKernel)\n");
        return EXIT_FAILURE;
    }

    status = clReleaseProgram(program_gradsmooth);
    return EXIT_SUCCESS;
}

std::string CLRuntime::getInfo(int j) const {
    cl_int status;//print some info
    string tmp1;
    char buffer[210];
    snprintf(buffer, sizeof(buffer), "num of GPU device find from context: %d \n", deviceListSize);
    LOGW("num of GPU device find from context: %d \n",
         deviceListSize);
    tmp1.append(std::string(buffer));
    //Query the name of the device
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_NAME, 100, buffer, NULL);
    snprintf(buffer, sizeof(buffer), " Device Name:%s \n", buffer);
    LOGW(" Device Name:%s \n", buffer);
    tmp1.append(std::string(buffer));

    //Query device type
    cl_device_type dev_type;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &dev_type,
                             NULL);
    snprintf(buffer, sizeof(buffer), "Device type: %lu ", dev_type);
    tmp1 += std::string(buffer);

    if (dev_type == CL_DEVICE_TYPE_CPU) {
        tmp1.append("CL_DEVICE_TYPE_CPU \n");
    } else if (dev_type == CL_DEVICE_TYPE_GPU) {
        tmp1.append("CL_DEVICE_TYPE_GPU\n");
    } else if (dev_type == CL_DEVICE_TYPE_ACCELERATOR) {
        tmp1.append("CL_DEVICE_TYPE_ACCELERATOR\n");
    } else if (dev_type == CL_DEVICE_TYPE_DEFAULT) {
        tmp1.append("CL_DEVICE_TYPE_DEFAULT\n");
    }

    //Query the maximum number of computing units of the device
    cl_uint UnitNum;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &UnitNum,
                             NULL);

    snprintf(buffer, sizeof(buffer), "Compute Units Number: %d\n", UnitNum);
    tmp1.append(std::string(buffer));
    //Query device core frequency
    cl_uint frequency;
    status =
            clGetDeviceInfo(listDevice[j], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint),
                            &frequency,
                            NULL);
    snprintf(buffer, sizeof(buffer), "Device Frequency: %d(MHz)\n", frequency);
    tmp1.append(std::string(buffer));
    // Query device global memory size
    cl_ulong GlobalSize;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong),
                             &GlobalSize,
                             NULL);
    snprintf(buffer, sizeof(buffer), "Device Global Mem Size: %0.0f(MB)\n",
             (float) GlobalSize / 1024 / 1024);

    tmp1.append(std::string(buffer));
    //Query device global memory cache line
    cl_uint GlobalCacheLine;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint),
                             &GlobalCacheLine, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global Mem CacheLine: %d(Byte)\n", GlobalCacheLine);
    tmp1.append(std::string(buffer));
    //Query the largest working group of equipment
    cl_ulong GlobalWGSize;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong),
                             &GlobalWGSize, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global MAX_WORK_GROUP Size: %lu\n", GlobalWGSize);
    tmp1.append(std::string(buffer));
    //Query the largest working group of equipment
    cl_ulong GlobalWISize;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(cl_ulong),
                             &GlobalWISize, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global MAX_WORK_ITEM Size: %lu\n", GlobalWISize);
    tmp1.append(std::string(buffer));

    cl_ulong LOCAL_MEM_SIZE;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong),
                             &LOCAL_MEM_SIZE, NULL);
    snprintf(buffer, sizeof(buffer), "Device Global LOCAL_MEM_SIZE Size: %0.0f(KB)\n",
             (float) LOCAL_MEM_SIZE / 1024);
    tmp1.append(string(buffer));

    //Query the OpenCL version supported by the device
    char DeviceVersion[110];
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_VERSION, 100, DeviceVersion, NULL);
    snprintf(buffer, sizeof(buffer), "Device Version:%s\n", DeviceVersion);
    tmp1.append(std::string(buffer));
    //Query device extension
    char *DeviceExtensions;
    size_t ExtenNum;
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_EXTENSIONS, 0, NULL, &ExtenNum);
    DeviceExtensions = (char *) malloc(ExtenNum);
    status = clGetDeviceInfo(listDevice[j], CL_DEVICE_EXTENSIONS, ExtenNum, DeviceExtensions, NULL);
    snprintf(buffer, sizeof(buffer), "Device Extensions: %s\n", DeviceExtensions);
    tmp1.append(std::string(buffer));
    return tmp1;
}

CLRuntime::~CLRuntime() {
//    LOGE( "***CLRuntime 0 resource released! ***\n");
    if (listDevice != NULL) {
        clReleaseKernel(kernel_gradsmooth);
        //release context
        clReleaseContext(gpu_context);
        free(listDevice);
    }

#if DEBUG_PRINT
    LOGE("***CLRuntime resource released! ***\n");
#endif
}

std::string CLRuntime::getInfoSt2(int jin) const {

    int i, j;
    string tmp1;
    cl_uint platformCount;
    cl_platform_id *platforms;
    cl_uint deviceCount;
    cl_device_id *devices;
    cl_uint maxComputeUnits;

    // get all platforms
    clGetPlatformIDs(0, NULL, &platformCount);

    platforms = (cl_platform_id *) malloc(sizeof(cl_platform_id) * platformCount);
    clGetPlatformIDs(platformCount, platforms, NULL);

    LOGW("Platform Count %d. \n", platformCount);

    for (i = 0; i < platformCount; i++) {

        // get all devices
        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, 0, NULL, &deviceCount);
        devices = (cl_device_id *) malloc(sizeof(cl_device_id) * deviceCount);

        clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, deviceCount, devices, NULL);

        // for each device print critical attributes
        for (j = 0; j < deviceCount; j++) {
            char *value;
            size_t valueSize;
            string tmpa = string(getCLDeviceName(i, j, devices[j]));
            LOGW(" TEST : %s  pos Q %d %d \n",
                 tmpa.c_str(),
                 tmpa.rfind("QUALCOMM"),
                 (tmpa.rfind("QUALCOMM") == std::string::npos));
            // print hardware device version
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, 0, NULL, &valueSize);
            value = (char *) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_VERSION, valueSize, value, NULL);
            LOGW(" %d.%d Hardware version: %s\n", j + 1,
                 1, value);
            free(value);

            // print software driver version
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, 0, NULL, &valueSize);
            value = (char *) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DRIVER_VERSION, valueSize, value, NULL);
            LOGW(" %d.%d Software version: %s\n", j + 1,
                 2, value);
            free(value);

            // print c version supported by compiler for device
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, 0, NULL, &valueSize);
            value = (char *) malloc(valueSize);
            clGetDeviceInfo(devices[j], CL_DEVICE_OPENCL_C_VERSION, valueSize, value, NULL);
            LOGW(" %d.%d OpenCL C version: %s\n", j + 1,
                 3, value);
            free(value);

            // print parallel compute units
            clGetDeviceInfo(devices[j], CL_DEVICE_MAX_COMPUTE_UNITS,
                            sizeof(maxComputeUnits), &maxComputeUnits, NULL);
            LOGW(" %d.%d Parallel compute units: %d\n",
                 j + 1, 4, maxComputeUnits);

            cl_device_type dev_type;
            clGetDeviceInfo(devices[j], CL_DEVICE_TYPE, sizeof(cl_device_type), &dev_type, NULL);

            std::string tmp1 = std::string("   Device type: ");
            if (dev_type == CL_DEVICE_TYPE_CPU) {
                tmp1.append("CL_DEVICE_TYPE_CPU \n");
            } else if (dev_type == CL_DEVICE_TYPE_GPU) {
                tmp1.append("CL_DEVICE_TYPE_GPU\n");
            } else if (dev_type == CL_DEVICE_TYPE_ACCELERATOR) {
                tmp1.append("CL_DEVICE_TYPE_ACCELERATOR\n");
            } else if (dev_type == CL_DEVICE_TYPE_DEFAULT) {
                tmp1.append("CL_DEVICE_TYPE_DEFAULT\n");
            }
            LOGW(" %d.%d  %s \n",
                 j + 1, 5, tmp1.c_str());
        }

        free(devices);

    }

    free(platforms);

    return std::string();
}

std::string CLRuntime::getCLDeviceName(int i,
                                       int j,
                                       cl_device_id device) const {// print device name
    std::string tmp1;
    char *value;
    size_t valueSize;
    clGetDeviceInfo(device, CL_DEVICE_NAME, 0, NULL, &valueSize);
    value = (char *) malloc(valueSize);
    clGetDeviceInfo(device, CL_DEVICE_NAME, valueSize, value, NULL);
    tmp1.append(std::string(value));
    LOGW("With platform %d. %d. Device: %s\n", i, j + 1, tmp1.c_str());
    free(value);
    return tmp1;
}

int cL_gradsmooth(Mat &depth_img, Mat &grad,
                  const int &thresh, const int &Radius, CLRuntime &bok_cl_runtime) {
    cl_int status = 0;
#if DEBUG_TIME
    long t1,t2;
    t1 = getCPUTickCount();
#endif

    const unsigned long src_width = depth_img.cols;
    const unsigned long src_height = depth_img.rows;
    //cout<<"srcImg2 size: "<<srcImg_2.cols <<" * "<<srcImg_2.rows<<endl;


    unsigned char *grad_ptr = grad.data;
    unsigned char *src_ptr = depth_img.data;
    Mat dst_img = Mat(depth_img.size(), CV_8UC1);
    //Mat blurredImg_2 = srcImg_2.clone();
    unsigned char *dst = dst_img.data;
    //memset(outbuffer, 0, 4 * 4 * 4);

#if DEBUG_TIME
    t2 = getCPUTickCount();
    cout << "prepair data takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;

    t1 = getCPUTickCount();
#endif
    cl_mem src_buffer = clCreateBuffer(
            bok_cl_runtime.gpu_context,
            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            sizeof(unsigned char) * src_width * src_height,
            src_ptr,
            &status);
    cl_mem grad_buffer = clCreateBuffer(
            bok_cl_runtime.gpu_context,
            CL_MEM_READ_ONLY | CL_MEM_USE_HOST_PTR,
            sizeof(unsigned char) * src_width * src_height,
            grad_ptr,
            &status);

    cl_mem dst_buffer = clCreateBuffer(
            bok_cl_runtime.gpu_context,
            CL_MEM_WRITE_ONLY | CL_MEM_ALLOC_HOST_PTR,
            sizeof(unsigned char) * src_width * src_height,
            NULL,
            &status);

#if DEBUG_TIME
    t2 = getCPUTickCount();
    cout << "clCreateBuffer takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
#endif

    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Create Buffer, outputBuffer. (clCreateBuffer)\n");
    }

    //t1 = getCPUTickCount();
    unsigned int arg_id = 0;
    status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_mem),
                            (void *) &src_buffer);
    status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_mem),
                            (void *) &grad_buffer);
    status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_mem),
                            (void *) &dst_buffer);
    status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_int),
                            (void *) &thresh);
    status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_int),
                            (void *) &Radius);
    //t2 = getCPUTickCount();
    //cout << "clSetKernelArg takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;

    //t1 = getCPUTickCount();
    cl_command_queue commandQueue = clCreateCommandQueue(bok_cl_runtime.gpu_context,
                                                         bok_cl_runtime.listDevice[0],
                                                         0,
                                                         &status);
    //t2 = getCPUTickCount();
    //cout << "clCreateCommandQueue takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
    if (status != CL_SUCCESS) {
        LOGE(
                "Error: Create Command Queue. (clCreateCommandQueue)\n");
    }

#if DEBUG_TIME
    t1 = getCPUTickCount();
#endif
    cl_uint work_dim = 2;
    size_t globalThreads[] = {src_width, src_height};
    //size_t localThreads[] = {32, 12};

    status = clEnqueueNDRangeKernel(commandQueue, bok_cl_runtime.kernel_gradsmooth,
                                    work_dim, NULL, globalThreads,
                                    NULL, 0,
                                    NULL, NULL);
    if (status != CL_SUCCESS) {
        LOGE("Error: Enqueueing kernel err_code:%d \n",
             status);
    }

    status = clFinish(commandQueue);
    if (status != CL_SUCCESS) {
        LOGE("Error: Finish command queue\n");
    }

#if DEBUG_TIME
    t2 = getCPUTickCount();
    cout << "clEnqueueNDRangeKernel & clFinish takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
#endif

#if DEBUG_TIME
    t1 = getCPUTickCount();
#endif
    status = clEnqueueReadBuffer(commandQueue,
                                 dst_buffer, CL_TRUE, 0,
                                 sizeof(unsigned char) * src_width * src_height, dst, 0, NULL,
                                 NULL);
#if DEBUG_TIME
    t2 = getCPUTickCount();
    cout << "clEnqueueReadBuffer takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
#endif

    if (status != CL_SUCCESS) {
        LOGE("Error: Read buffer queue err_code:%d \n",
             status);
    }

#if DEBUG_TIME
    t1 = getCPUTickCount();
#endif

    depth_img = dst_img.clone();

#if DEBUG_TIME
    t2 = getCPUTickCount();
    cout << "clone rect takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
#endif

#if DEBUG_TIME
    t1 = getCPUTickCount();
#endif

    status = clReleaseMemObject(src_buffer);
    status = clReleaseMemObject(grad_buffer);
    status = clReleaseMemObject(dst_buffer);
    status = clReleaseCommandQueue(commandQueue);

#if DEBUG_TIME
    t2 = getCPUTickCount();
    cout << "ReleaseMem takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
#endif

    return 0;
}


