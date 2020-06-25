
#include <jni.h>
#include <string>
#include "android_log.h"
#include "opencl_cl_files.h"
#include <CLbackend.h>
#include <opencv2/opencv.hpp>


using namespace std;
using namespace cv;

CLRuntime *m_runtime = new CLRuntime();


extern "C" JNIEXPORT jstring
JNICALL
Java_zt_mezon_graphomany_openclsandbox_ui_mactivity_MainViewNDKHelper_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello;
////    for (int i = 0; i <tmp->deviceListSize ; ++i) {
//  int itmp = tmp->init();
//  LOG_W2(ANDROID_LOG_DEBUG, "%d = tmp->init() ", itmp);
//  LOG(ANDROID_LOG_DEBUG, "%d = tmp->init() ", itmp);
//  if (itmp == CL_SUCCESS) {
//
//    // Load kernel program, compile CL program, generate CL kernel instance
//    hello.append( m_runtime->getInfo(0));

    hello.append(m_runtime->getInfo());
    LOGW("%s", hello.c_str());
//
//  } else { hello = std::string(" No Cl found in this system"); }
//  delete tmp;
    return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_zt_mezon_graphomany_openclsandbox_ui_mactivity_MainViewNDKHelper_coreFiltering(JNIEnv *env,
                                                                                    jobject thiz,
                                                                                    jlong mat_gray,
                                                                                    jint flag) {

    clock_t begin_total = clock();
    clock_t begin = clock();
    double totalTime = 0.0;
    // get Mat from raw address

    Mat &grayImg = *(Mat *) mat_gray;

    //  cv::cvtColor(grayImg, grayImg, cv::COLOR_RGBA2RGB);
    int width = grayImg.cols;
    int height = grayImg.rows;
    uchar *data = (uchar *) grayImg.data;

    LOGD("srcImg channels: %d", grayImg.channels());
    Mat dst(height, width, CV_8UC4);
    uchar *buffer = new uchar[width * height * 4];

    cl_int error;

    cl_image_format clImageFormat;
    clImageFormat.image_channel_order = CL_RGBA;
    clImageFormat.image_channel_data_type = CL_UNORM_INT8;

    cl_image_desc clImageDesc;
    clImageDesc.image_type = CL_MEM_OBJECT_IMAGE2D;
    clImageDesc.image_width = width;
    clImageDesc.image_height = height;
    clImageDesc.image_row_pitch = 0;
    clImageDesc.image_slice_pitch = 0;
    clImageDesc.num_mip_levels = 0;
    clImageDesc.num_samples = 0;
    clImageDesc.buffer = NULL;

    size_t origin[3] = {0, 0, 0};  //Image origin to be read
    size_t region[3] = {(size_t) width, (size_t) height,
                        1};  //The image area to be read, the third parameter is depth = 1

    cl_mem srcImg =
            clCreateImage2D(m_runtime->m_context,
                            CL_MEM_READ_WRITE,
                            &clImageFormat,
                            clImageDesc.image_width,
                            clImageDesc.image_height,
                            clImageDesc.image_row_pitch,
                            NULL,
                            &error);
    CHECK_CL_SUCCESS(error, "Failed to create srcImg");

    error =
            clEnqueueWriteImage(m_runtime->m_cmd_queue,
                                srcImg,
                                CL_TRUE,
                                origin,
                                region,
                                0,
                                0,
                                data,
                                0,
                                NULL,
                                NULL);

    clFinish(m_runtime->m_cmd_queue);
    CHECK_CL_SUCCESS(error, "Failed to write srcImg");


    cl_mem dstImg = clCreateImage2D(m_runtime->m_context,
                                    CL_MEM_WRITE_ONLY,
                                    &clImageFormat,
                                    clImageDesc.image_width,
                                    clImageDesc.image_height,
                                    clImageDesc.image_row_pitch,
                                    NULL,
                                    &error);

    if (error != CL_SUCCESS) {
        LOGD("Failed to create dstImg ");
    } else {
        LOGD("Success to create dstImg ");
    }

    size_t globalThreads[] =
            {static_cast<size_t>(ceil(width / 16) * 16),
             static_cast<size_t>(ceil(height / 16) * 16),};
    size_t localThreads[] = {16, 16};

    if (error != CL_SUCCESS) {
        LOGD("Failed to create kernel");
    } else {
        LOGD("Success to create kernel");
    }

    error = clSetKernelArg(m_runtime->mKErnelSobelFilter, 0, sizeof(cl_mem), &srcImg);
    error |= clSetKernelArg(m_runtime->mKErnelSobelFilter, 1, sizeof(cl_mem), &dstImg);
    error |= clSetKernelArg(m_runtime->mKErnelSobelFilter, 2, sizeof(cl_int), &width);
    error |= clSetKernelArg(m_runtime->mKErnelSobelFilter, 3, sizeof(cl_int), &height);

    if (error != CL_SUCCESS) {
        LOGD("Error setting kernel arguments");
    } else {
        LOGD("Successfully setting kernel arguments");
    }
    begin = clock();
    error = clEnqueueNDRangeKernel(m_runtime->m_cmd_queue,
                                   m_runtime->mKErnelSobelFilter,
                                   2,
                                   NULL,
                                   globalThreads,
                                   localThreads,
                                   0,
                                   NULL,
                                   NULL);
    clFinish(m_runtime->m_cmd_queue);

    if (error != CL_SUCCESS) {
        LOGD("Error queueing kernel for execution");
    } else {
        LOGD("Successfully queueing kernel for execution");
    }

    totalTime = double(clock() - begin_total) / CLOCKS_PER_SEC;
    LOG_W2(ANDROID_LOG_INFO, "Execution time slice = %f seconds\n",
           totalTime);

    error = clEnqueueReadImage(m_runtime->m_cmd_queue,
                               dstImg,
                               CL_TRUE,
                               origin,
                               region,
                               0,
                               0,
                               buffer,
                               0,
                               NULL,
                               NULL);

    if (error != CL_SUCCESS) {
        LOGD("Error to read dstImg srcImg");
    } else {
        LOGD("Successfully to read dstImg srcImg");
    }

    dst.data = (uchar *) buffer;
    dst.convertTo(dst, CV_8UC4);
    // cv::cvtColor(dst,dst, CV_RGBA2GRAY);
    grayImg = dst;

    LOGD("Size in JNI: %d X %d", dst.cols, dst.rows);

    clReleaseMemObject(srcImg);
    clReleaseMemObject(dstImg);

    totalTime = double(clock() - begin_total) / CLOCKS_PER_SEC;
    LOG_W2(ANDROID_LOG_INFO, "Total time slice = %f seconds\n",
           totalTime);

}extern "C"
JNIEXPORT void JNICALL
Java_zt_mezon_graphomany_openclsandbox_ui_mactivity_MainViewNDKHelper_initCL(JNIEnv *env,
                                                                             jobject thiz,
                                                                             jint width,
                                                                             jint height) {
    cl_int error;
    cl_int status = CL_SUCCESS;
    NNM::OpenCLSymbolsOperator::createOpenCLSymbolsOperatorSingleInstance();
    if (nullptr == NNM::OpenCLSymbolsOperator::getOpenclSymbolsPtr()) {
        LOGD("OpenCL init error , callback ...");
        EXIT_FAILURE;
    } else {
        LOGD("OpenCL init ok");
    }

    cl_int ret = clGetPlatformIDs(1, &m_runtime->platformId, &m_runtime->retNumPlatforms);
    ret = clGetDeviceIDs(m_runtime->platformId, CL_DEVICE_TYPE_DEFAULT, 1, &m_runtime->m_device,
                         &m_runtime->retNumDevices);

    m_runtime->m_context = clCreateContext(NULL, 1, &m_runtime->m_device, NULL, NULL, &error);

    if (error != CL_SUCCESS) {
        LOGE("Can't create a valid OpenCL context");

    } else {
        LOGD("Create a valid OpenCL context");
    }

    char str_buffer[1024];

    // Get platform name
    error = clGetPlatformInfo(m_runtime->platformId, CL_PLATFORM_NAME, sizeof(str_buffer),
                              &str_buffer, NULL);
    LOGD("CL_PLATFORM_NAME: %s", str_buffer);
    // Get platform vendor
    error = clGetPlatformInfo(m_runtime->platformId, CL_PLATFORM_VENDOR, sizeof(str_buffer),
                              &str_buffer, NULL);
    LOGD("CL_PLATFORM_VENDOR: %s", str_buffer);
    // Get platform OpenCL version
    error = clGetPlatformInfo(m_runtime->platformId, CL_PLATFORM_VERSION, sizeof(str_buffer),
                              &str_buffer, NULL);
    LOGD("CL_PLATFORM_VERSION: %s", str_buffer);
    // Get platform OpenCL profile
    error = clGetPlatformInfo(m_runtime->platformId, CL_PLATFORM_PROFILE, sizeof(str_buffer),
                              &str_buffer, NULL);
    LOGD("CL_PLATFORM_PROFILE: %s", str_buffer);
    // Get platform OpenCL supported extensions
    error =
            clGetPlatformInfo(m_runtime->platformId, CL_PLATFORM_EXTENSIONS, sizeof(str_buffer),
                              &str_buffer, NULL);
    LOGD("CL_PLATFORM_EXTENSIONS: %s", str_buffer);

    error = clGetDeviceInfo(m_runtime->m_device, CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer,
                            NULL);
    LOGD(" CL_DEVICE_NAME: %s", str_buffer);
    // Get device hardware version
    error = clGetDeviceInfo(m_runtime->m_device, CL_DEVICE_VERSION, sizeof(str_buffer),
                            &str_buffer, NULL);
    LOGD("  CL_DEVICE_VERSION: %s", str_buffer);
    // Get device software version
    error = clGetDeviceInfo(m_runtime->m_device, CL_DRIVER_VERSION, sizeof(str_buffer),
                            &str_buffer, NULL);
    LOGD(" CL_DRIVER_VERSION:  %s", str_buffer);
    // Get device OpenCL C version
    error =
            clGetDeviceInfo(m_runtime->m_device, CL_DEVICE_OPENCL_C_VERSION, sizeof(str_buffer),
                            &str_buffer, NULL);
    LOGD("CL_DEVICE_OPENCL_C_VERSION: %s", str_buffer);

    cl_bool imageSupport = CL_FALSE;
    error = clGetDeviceInfo(m_runtime->m_device, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool),
                            &imageSupport, NULL);
    if (imageSupport != CL_TRUE) {
        LOGE("OpenCL device does not support images.");
    } else {
        LOGD("OpenCL device support images.");
    }

    m_runtime->m_cmd_queue = clCreateCommandQueue(m_runtime->m_context, m_runtime->m_device, 0,
                                                  &error);

    cl_int err = 0;
    m_runtime->mProgram =
            clCreateProgramWithSource(m_runtime->m_context,
                                      (cl_uint) CL_SOBEL_COLOR_SIZE,
                                      CL_SOBEL_COLOR,
                                      NULL,
                                      &err);
    if (err != CL_SUCCESS) {
        LOG_W2(ANDROID_LOG_ERROR, "  with clCreateProgramWithSource. << %d \n",
               err);
    }
//https://www.khronos.org/registry/OpenCL/sdk/1.0/docs/man/xhtml/clBuildProgram.html cl_khr_fp16
    err = clBuildProgram(m_runtime->mProgram,
                         0,
                         NULL,
                         "-cl-mad-enable -cl-fast-relaxed-math -cl-single-precision-constant",
                         NULL,
                         NULL);
    if (err != CL_SUCCESS) {
        LOG_W2(ANDROID_LOG_ERROR, " with clBuildProgram %d \n", err);

        size_t len = 0;
        err = clGetProgramBuildInfo(m_runtime->mProgram, m_runtime->m_device, CL_PROGRAM_BUILD_LOG,
                                    0, NULL, &len);
        char log[len];
        log[0] = 0;
        err = clGetProgramBuildInfo(m_runtime->mProgram, m_runtime->m_device, CL_PROGRAM_BUILD_LOG,
                                    len, log, NULL);
        if (err == CL_INVALID_VALUE) {
            LOG_W2(ANDROID_LOG_ERROR,
                   " There was a build error, but there is insufficient space allocated to show the build logs. \n");
        } else {
            LOG_W2(ANDROID_LOG_ERROR, " There was a build error, Build error:. %s \n", log);

        }
    }
    if (error != CL_SUCCESS) {
        LOGD("Failed to create program");
    } else {
        LOGD("Success to create program");
    }
    m_runtime->mKErnelSobelFilter = clCreateKernel(m_runtime->mProgram, "sobel_filter_color",
                                                   &error);
    char *program_log;
    const char options[] = "";
    size_t log_size;

    error = clBuildProgram(m_runtime->mProgram, 1, &m_runtime->m_device, options, NULL, NULL);

    if (error != CL_SUCCESS) {
        size_t len;
        char buffer[8 * 1024];

        LOGD("Error: Failed to build program executable!\n");
        clGetProgramBuildInfo(m_runtime->mProgram,
                              m_runtime->m_device,
                              CL_PROGRAM_BUILD_LOG,
                              sizeof(buffer),
                              buffer,
                              &len);
        LOGD("build program %s\n", buffer);
    }

    if (error != CL_SUCCESS) {
        LOGD("Failed to build program");
    } else {
        LOGD("Success to build program");
    }

}