
#include <jni.h>
#include <string>
#include "android_log.h"
#include "opencl_cl_files.h"
#include <OpenCLWrapper.h>
#include <opencv2/opencv.hpp>



using namespace std;
using namespace cv;

cl_platform_id platformId = NULL;
cl_device_id mDeviceId = NULL;
cl_context mContext = NULL;
cl_command_queue mCommandQueue = NULL;
cl_uint retNumDevices;
cl_uint retNumPlatforms;
cl_program mProgram = NULL;
cl_kernel mKErnelSobelFilter;

const char *getErrorString(cl_int error) {
  switch (error) {
// run-time and JIT compiler errors
    case 0: return "CL_SUCCESS";
    case -1: return "CL_DEVICE_NOT_FOUND";
    case -2: return "CL_DEVICE_NOT_AVAILABLE";
    case -3: return "CL_COMPILER_NOT_AVAILABLE";
    case -4: return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
    case -5: return "CL_OUT_OF_RESOURCES";
    case -6: return "CL_OUT_OF_HOST_MEMORY";
    case -7: return "CL_PROFILING_INFO_NOT_AVAILABLE";
    case -8: return "CL_MEM_COPY_OVERLAP";
    case -9: return "CL_IMAGE_FORMAT_MISMATCH";
    case -10: return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
    case -11: return "CL_BUILD_PROGRAM_FAILURE";
    case -12: return "CL_MAP_FAILURE";
    case -13: return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
    case -14: return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
    case -15: return "CL_COMPILE_PROGRAM_FAILURE";
    case -16: return "CL_LINKER_NOT_AVAILABLE";
    case -17: return "CL_LINK_PROGRAM_FAILURE";
    case -18: return "CL_DEVICE_PARTITION_FAILED";
    case -19: return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

// compile-time errors
    case -30: return "CL_INVALID_VALUE";
    case -31: return "CL_INVALID_DEVICE_TYPE";
    case -32: return "CL_INVALID_PLATFORM";
    case -33: return "CL_INVALID_DEVICE";
    case -34: return "CL_INVALID_CONTEXT";
    case -35: return "CL_INVALID_QUEUE_PROPERTIES";
    case -36: return "CL_INVALID_COMMAND_QUEUE";
    case -37: return "CL_INVALID_HOST_PTR";
    case -38: return "CL_INVALID_MEM_OBJECT";
    case -39: return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
    case -40: return "CL_INVALID_IMAGE_SIZE";
    case -41: return "CL_INVALID_SAMPLER";
    case -42: return "CL_INVALID_BINARY";
    case -43: return "CL_INVALID_BUILD_OPTIONS";
    case -44: return "CL_INVALID_PROGRAM";
    case -45: return "CL_INVALID_PROGRAM_EXECUTABLE";
    case -46: return "CL_INVALID_KERNEL_NAME";
    case -47: return "CL_INVALID_KERNEL_DEFINITION";
    case -48: return "CL_INVALID_KERNEL";
    case -49: return "CL_INVALID_ARG_INDEX";
    case -50: return "CL_INVALID_ARG_VALUE";
    case -51: return "CL_INVALID_ARG_SIZE";
    case -52: return "CL_INVALID_KERNEL_ARGS";
    case -53: return "CL_INVALID_WORK_DIMENSION";
    case -54: return "CL_INVALID_WORK_GROUP_SIZE";
    case -55: return "CL_INVALID_WORK_ITEM_SIZE";
    case -56: return "CL_INVALID_GLOBAL_OFFSET";
    case -57: return "CL_INVALID_EVENT_WAIT_LIST";
    case -58: return "CL_INVALID_EVENT";
    case -59: return "CL_INVALID_OPERATION";
    case -60: return "CL_INVALID_GL_OBJECT";
    case -61: return "CL_INVALID_BUFFER_SIZE";
    case -62: return "CL_INVALID_MIP_LEVEL";
    case -63: return "CL_INVALID_GLOBAL_WORK_SIZE";
    case -64: return "CL_INVALID_PROPERTY";
    case -65: return "CL_INVALID_IMAGE_DESCRIPTOR";
    case -66: return "CL_INVALID_COMPILER_OPTIONS";
    case -67: return "CL_INVALID_LINKER_OPTIONS";
    case -68: return "CL_INVALID_DEVICE_PARTITION_COUNT";

// extension errors
    case -1000: return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
    case -1001: return "CL_PLATFORM_NOT_FOUND_KHR";
    case -1002: return "CL_INVALID_D3D10_DEVICE_KHR";
    case -1003: return "CL_INVALID_D3D10_RESOURCE_KHR";
    case -1004: return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
    case -1005: return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
    default: return "Unknown OpenCL error";
  }
}
#define ASSERT_CL_SUCCESS(error, x...)                  \
    if (error != CL_SUCCESS) {                       \
        LOG_W2(ANDROID_LOG_ERROR, "Assertion CL_SUCCESS failed %s %d %s\n", x, err, getErrorString((err)) );\
        std::exit(err);\
    }

#define CHECK_CL_SUCCESS(err, x...)                  \
    if (err != CL_SUCCESS) {                       \
        LOG_W2(ANDROID_LOG_ERROR, "%s %d %s\n", x, err, getErrorString((err)) );\
    }
extern "C" JNIEXPORT jstring
JNICALL
Java_zt_mezon_graphomany_openclsandbox_MainActivity_stringFromJNI(
    JNIEnv *env,
    jobject /* this */) {
//  CLRuntime *tmp = new CLRuntime();
  std::string hello;
////    for (int i = 0; i <tmp->deviceListSize ; ++i) {
//  int itmp = tmp->init();
//  LOG_W2(ANDROID_LOG_DEBUG, "%d = tmp->init() ", itmp);
//  LOG(ANDROID_LOG_DEBUG, "%d = tmp->init() ", itmp);
//  if (itmp == CL_SUCCESS) {
//
//    // Load kernel program, compile CL program, generate CL kernel instance
//    hello.append(tmp->getInfo(0));
//    LOGW("%s", hello.c_str());
//    tmp->getInfoSt2(0);
//
//  } else { hello = std::string(" No Cl found in this system"); }
//  delete tmp;
  return env->NewStringUTF(hello.c_str());
}
extern "C"
JNIEXPORT void JNICALL
Java_zt_mezon_graphomany_openclsandbox_MainActivity_coreFiltering(JNIEnv *env,
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
      clCreateImage2D(mContext,
                      CL_MEM_READ_WRITE,
                      &clImageFormat,
                      clImageDesc.image_width,
                      clImageDesc.image_height,
                      clImageDesc.image_row_pitch,
                      NULL,
                      &error);
  CHECK_CL_SUCCESS(error,"Failed to create srcImg");

  error =
      clEnqueueWriteImage(mCommandQueue,
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

  clFinish(mCommandQueue);
  CHECK_CL_SUCCESS(error,"Failed to write srcImg");


  cl_mem dstImg = clCreateImage2D(mContext,
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
      {static_cast<size_t>(ceil(width / 16) * 16), static_cast<size_t>(ceil(height / 16) * 16),};
  size_t localThreads[] = {16, 16};

  if (error != CL_SUCCESS) {
    LOGD("Failed to create kernel");
  } else {
    LOGD("Success to create kernel");
  }

  error = clSetKernelArg(mKErnelSobelFilter, 0, sizeof(cl_mem), &srcImg);
  error |= clSetKernelArg(mKErnelSobelFilter, 1, sizeof(cl_mem), &dstImg);
  error |= clSetKernelArg(mKErnelSobelFilter, 2, sizeof(cl_int), &width);
  error |= clSetKernelArg(mKErnelSobelFilter, 3, sizeof(cl_int), &height);

  if (error != CL_SUCCESS) {
    LOGD("Error setting kernel arguments");
  } else {
    LOGD("Successfully setting kernel arguments");
  }
  begin = clock();
  error = clEnqueueNDRangeKernel(mCommandQueue,
                                 mKErnelSobelFilter,
                                 2,
                                 NULL,
                                 globalThreads,
                                 localThreads,
                                 0,
                                 NULL,
                                 NULL);
  clFinish(mCommandQueue);

  if (error != CL_SUCCESS) {
    LOGD("Error queueing kernel for execution");
  } else {
    LOGD("Successfully queueing kernel for execution");
  }

  totalTime = double(clock() - begin_total) / CLOCKS_PER_SEC;
  LOG_W2(ANDROID_LOG_INFO, "Execution time slice = %f seconds\n",
         totalTime);

  error = clEnqueueReadImage(mCommandQueue,
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
Java_zt_mezon_graphomany_openclsandbox_MainActivity_initCL(JNIEnv *env,
                                                                 jobject thiz,
                                                                 jint width,
                                                                 jint height)  {
  cl_int error;
  cl_int status = CL_SUCCESS;
  NNM::OpenCLSymbolsOperator::createOpenCLSymbolsOperatorSingleInstance();
  if (nullptr == NNM::OpenCLSymbolsOperator::getOpenclSymbolsPtr()) {
    LOGD("OpenCL init error , callback ...");
    EXIT_FAILURE;
  } else {
    LOGD("OpenCL init ok");
  }

  cl_int ret = clGetPlatformIDs(1, &platformId, &retNumPlatforms);
  ret = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_DEFAULT, 1, &mDeviceId, &retNumDevices);

  mContext = clCreateContext(NULL, 1, &mDeviceId, NULL, NULL, &error);

  if (error != CL_SUCCESS) {
    LOGE("Can't create a valid OpenCL context");

  } else {
    LOGD("Create a valid OpenCL context");
  }

  char str_buffer[1024];

  // Get platform name
  error = clGetPlatformInfo(platformId, CL_PLATFORM_NAME, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_PLATFORM_NAME: %s", str_buffer);
  // Get platform vendor
  error = clGetPlatformInfo(platformId, CL_PLATFORM_VENDOR, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_PLATFORM_VENDOR: %s", str_buffer);
  // Get platform OpenCL version
  error = clGetPlatformInfo(platformId, CL_PLATFORM_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_PLATFORM_VERSION: %s", str_buffer);
  // Get platform OpenCL profile
  error = clGetPlatformInfo(platformId, CL_PLATFORM_PROFILE, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_PLATFORM_PROFILE: %s", str_buffer);
  // Get platform OpenCL supported extensions
  error =
      clGetPlatformInfo(platformId, CL_PLATFORM_EXTENSIONS, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_PLATFORM_EXTENSIONS: %s", str_buffer);

  error = clGetDeviceInfo(mDeviceId, CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
  LOGD(" CL_DEVICE_NAME: %s", str_buffer);
  // Get device hardware version
  error = clGetDeviceInfo(mDeviceId, CL_DEVICE_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("  CL_DEVICE_VERSION: %s", str_buffer);
  // Get device software version
  error = clGetDeviceInfo(mDeviceId, CL_DRIVER_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD(" CL_DRIVER_VERSION:  %s", str_buffer);
  // Get device OpenCL C version
  error =
      clGetDeviceInfo(mDeviceId, CL_DEVICE_OPENCL_C_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_DEVICE_OPENCL_C_VERSION: %s", str_buffer);

  cl_bool imageSupport = CL_FALSE;
  error = clGetDeviceInfo(mDeviceId, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &imageSupport, NULL);
  if (imageSupport != CL_TRUE) {
    LOGE("OpenCL device does not support images.");
  } else {
    LOGD("OpenCL device support images.");
  }

  mCommandQueue = clCreateCommandQueue(mContext, mDeviceId, 0, &error);

  cl_int err = 0;
  mProgram =
      clCreateProgramWithSource(mContext,
                                (cl_uint) CL_SOBEL_COLOR_SIZE,
                                CL_SOBEL_COLOR,
                                NULL,
                                &err);
  if (err != CL_SUCCESS) {
    LOG_W2(ANDROID_LOG_ERROR, "  with clCreateProgramWithSource. << %d \n",
           err);
  }
//https://www.khronos.org/registry/OpenCL/sdk/1.0/docs/man/xhtml/clBuildProgram.html cl_khr_fp16
  err = clBuildProgram(mProgram,
                       0,
                       NULL,
                       "-cl-mad-enable -cl-fast-relaxed-math -cl-single-precision-constant",
                       NULL,
                       NULL);
  if (err != CL_SUCCESS) {
    LOG_W2(ANDROID_LOG_ERROR, " with clBuildProgram %d \n", err);

    size_t len = 0;
    err = clGetProgramBuildInfo(mProgram, mDeviceId, CL_PROGRAM_BUILD_LOG, 0, NULL, &len);
    char log[len];
    log[0] = 0;
    err = clGetProgramBuildInfo(mProgram, mDeviceId, CL_PROGRAM_BUILD_LOG, len, log, NULL);
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
  mKErnelSobelFilter = clCreateKernel(mProgram, "sobel_filter_color", &error);
  char *program_log;
  const char options[] = "";
  size_t log_size;

  error = clBuildProgram(mProgram, 1, &mDeviceId, options, NULL, NULL);

  if (error != CL_SUCCESS) {
    size_t len;
    char buffer[8 * 1024];

    LOGD("Error: Failed to build program executable!\n");
    clGetProgramBuildInfo(mProgram,
                          mDeviceId,
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