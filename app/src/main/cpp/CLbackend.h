#ifndef __CL_BACKEND_H
#define __CL_BACKEND_H

#include <string>
#include <time.h>
#include <opencv2/opencv.hpp>
#include <string.h>


#include "libopencl.h"

using namespace std;
using namespace cv;

#define USE_PAD2 0
#define USE_WORKGROUP 0



class CLRuntime{
public:
	CLRuntime();
	~CLRuntime();

	cl_platform_id platform ;
	cl_context gpu_context;
	cl_device_id *devices = NULL;
    size_t deviceListSize;
	cl_kernel kernel_gradsmooth ;
    string tmp;
    std::string getInfo(int j)const;
    std::string getInfoSt2(int j)const;

    int init();

	int iniKernelsgrandsmoothTest(cl_kernel& kernel_gradsmooth) const;
};

int cL_gradsmooth(Mat & depth_img, Mat & grad, 
						 const int& thresh, const int & Radius, CLRuntime &bok_cl_runtime );


#endif

