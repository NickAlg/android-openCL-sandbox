
#include <jni.h>
#include <string>
#include "android_log.h"
#include "opencl_cl_files.h"
#include <OpenCLWrapper.h>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

cl_platform_id platformId = NULL;
cl_device_id deviceID = NULL;
cl_context context = NULL;
cl_command_queue commandQueue = NULL;
cl_uint retNumDevices;
cl_uint retNumPlatforms;
cl_program program = NULL;

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_administrator_openclsandbox_MainActivity_stringFromJNI(
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
Java_com_example_administrator_openclsandbox_MainActivity_coreFiltering(JNIEnv *env,
                                                                        jobject thiz,
                                                                        jlong mat_gray,
                                                                        jint flag) {
  NNM::OpenCLSymbolsOperator::createOpenCLSymbolsOperatorSingleInstance();
  if (nullptr == NNM::OpenCLSymbolsOperator::getOpenclSymbolsPtr()) {
    LOGD("OpenCL init error , callback ...");
    EXIT_FAILURE;
  } else {
    LOGD("OpenCL init ok");
  }
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
  size_t region[3] = {(size_t) width, (size_t) height, 1};  //The image area to be read, the third parameter is depth = 1

  cl_mem srcImg =
      clCreateImage(context, CL_MEM_READ_WRITE, &clImageFormat, &clImageDesc, NULL, &error);
  if (error != CL_SUCCESS) {
    LOGD("Failed to create srcImg");
  } else {
    LOGD("Success to create srcImg");
  }
  error =
      clEnqueueWriteImage(commandQueue, srcImg, CL_TRUE, origin, region, 0, 0, data, 0, NULL, NULL);

  clFinish(commandQueue);
  if (error != CL_SUCCESS) {
    LOGD("Failed to write srcImg");
  } else {
    LOGD("Success to write  srcImg");
  }

  cl_mem dstImg =
      clCreateImage(context, CL_MEM_WRITE_ONLY, &clImageFormat, &clImageDesc, NULL, &error);

  if (error != CL_SUCCESS) {
    LOGD("Failed to create dstImg ");
  } else {
    LOGD("Success to create dstImg ");
  }

  size_t globalThreads[] =
      {static_cast<size_t>(ceil(width / 16) * 16), static_cast<size_t>(ceil(height / 16) * 16),};
  size_t localThreads[] = {16, 16};

  cl_kernel kernel = clCreateKernel(program, "sobel_filter_color", &error);

  if (error != CL_SUCCESS) {
    LOGD("Failed to create kernel");
  } else {
    LOGD("Success to create kernel");
  }

  error = clSetKernelArg(kernel, 0, sizeof(cl_mem), &srcImg);
  error |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &dstImg);
  error |= clSetKernelArg(kernel, 2, sizeof(cl_int), &width);
  error |= clSetKernelArg(kernel, 3, sizeof(cl_int), &height);

  if (error != CL_SUCCESS) {
    LOGD("Error setting kernel arguments");
  } else {
    LOGD("Successfully setting kernel arguments");
  }
  begin = clock();
  error = clEnqueueNDRangeKernel(commandQueue,
                                 kernel,
                                 2,
                                 NULL,
                                 globalThreads,
                                 localThreads,
                                 0,
                                 NULL,
                                 NULL);
  clFinish(commandQueue);

  if (error != CL_SUCCESS) {
    LOGD("Error queueing kernel for execution");
  } else {
    LOGD("Successfully queueing kernel for execution");
  }

  totalTime = double(clock() - begin_total) / CLOCKS_PER_SEC;
  LOG_W2(ANDROID_LOG_INFO, "Execution time slice = %f seconds\n",
         totalTime);

  error = clEnqueueReadImage(commandQueue,
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
Java_com_example_administrator_openclsandbox_MainActivity_initCL(JNIEnv *env, jobject thiz) {
  cl_int  error;
  cl_int status = CL_SUCCESS;
  NNM::OpenCLSymbolsOperator::createOpenCLSymbolsOperatorSingleInstance();
  if (nullptr == NNM::OpenCLSymbolsOperator::getOpenclSymbolsPtr()) {
    LOGD("OpenCL init error , callback ...");
    EXIT_FAILURE;
  } else {
    LOGD("OpenCL init ok");
  }

  cl_int ret = clGetPlatformIDs(1, &platformId, &retNumPlatforms);
  ret = clGetDeviceIDs(platformId, CL_DEVICE_TYPE_DEFAULT, 1, &deviceID, &retNumDevices);

  context = clCreateContext(NULL, 1, &deviceID, NULL, NULL,  &error);

  if(error != CL_SUCCESS) {
    LOGE("Can't create a valid OpenCL context");

  }else{
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
  error = clGetPlatformInfo(platformId, CL_PLATFORM_EXTENSIONS, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_PLATFORM_EXTENSIONS: %s", str_buffer);

  error = clGetDeviceInfo(deviceID, CL_DEVICE_NAME, sizeof(str_buffer), &str_buffer, NULL);
  LOGD(" CL_DEVICE_NAME: %s", str_buffer);
  // Get device hardware version
  error = clGetDeviceInfo(deviceID, CL_DEVICE_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("  CL_DEVICE_VERSION: %s", str_buffer);
  // Get device software version
  error = clGetDeviceInfo(deviceID, CL_DRIVER_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD(" CL_DRIVER_VERSION:  %s", str_buffer);
  // Get device OpenCL C version
  error = clGetDeviceInfo(deviceID, CL_DEVICE_OPENCL_C_VERSION, sizeof(str_buffer), &str_buffer, NULL);
  LOGD("CL_DEVICE_OPENCL_C_VERSION: %s", str_buffer);

  cl_bool imageSupport = CL_FALSE;
  error = clGetDeviceInfo(deviceID, CL_DEVICE_IMAGE_SUPPORT, sizeof(cl_bool), &imageSupport, NULL);
  if(imageSupport != CL_TRUE){
    LOGE("OpenCL device does not support images.");
  }else{
    LOGD("OpenCL device support images.");
  }

  commandQueue = clCreateCommandQueue(context, deviceID, 0, &error);



  program = clCreateProgramWithSource(context, 1, CL_SOBEL_COLOR, NULL, &error);
  if ( error != CL_SUCCESS){
    LOGD("Failed to create program");
  }else{
    LOGD("Success to create program");
  }

  char *program_log;
  const char options[] = "";
  size_t log_size;

  error = clBuildProgram(program, 1, &deviceID, options, NULL, NULL);

  if (error != CL_SUCCESS)
  {
    size_t len;
    char buffer[8 * 1024];

    LOGD("Error: Failed to build program executable!\n");
    clGetProgramBuildInfo(program, deviceID, CL_PROGRAM_BUILD_LOG, sizeof(buffer), buffer, &len);
    LOGD("build program %s\n", buffer);
  }

  if ( error != CL_SUCCESS){
    LOGD("Failed to build program");
  }else{
    LOGD("Success to build program");
  }


}