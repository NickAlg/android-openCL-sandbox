#ifndef __CL_BACKEND_H
#define __CL_BACKEND_H

#include <string>
#include <time.h>
#include <opencv2/opencv.hpp>
#include <string.h>

#include <OpenCLWrapper.h>

using namespace std;
using namespace cv;

#define USE_PAD2 0
#define USE_WORKGROUP 0

class CLRuntime {
 public:
  CLRuntime();
  ~CLRuntime();

  cl_platform_id listPlatform;
  cl_platform_id *listPlatforms;
  cl_context gpu_context;
  cl_device_id *listDevice = NULL;
  size_t deviceListSize;
  cl_kernel kernel_gradsmooth;
  cl_uint nDevice;
  cl_uint nPlatform;

  string tmp;
  std::string getInfo(int j) const;
  std::string getInfoSt2(int j) const;

  int init();

  int iniKernelsgrandsmoothTest(cl_kernel &kernel_gradsmooth) const;
  std::stringstream getStringstream(
      char *info,
      cl_uint nPlatform,
      cl_platform_id *listPlatform,
      cl_uint &nDevice,
      cl_device_id *listDevice);
  std::string getCLDeviceName(int i, int j, cl_device_id device) const;
};

int cL_gradsmooth(Mat &depth_img, Mat &grad,
                  const int &thresh, const int &Radius, CLRuntime &bok_cl_runtime);

#endif

