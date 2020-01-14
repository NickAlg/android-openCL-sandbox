#include "CLbackend.h"
#define DEBUG_PRINT 0
#define DEBUG_TIME 0

#define KERNEL(...)#__VA_ARGS__
 



const char *kernelSourceCode_grad_smootch = KERNEL(

__kernel void grad_smootch(__global uchar *src_buffer, 
						__global uchar *grad_buffer,
						__global uchar *dst_buffer,
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
    int dst_pixel_id = boky * work_width + bokx ;
	int grad_data = (int)grad_buffer[dst_pixel_id];
	
	if(grad_data < thresh )
		{
			int sumWeight = 0;
			int sumC0 = 0;
			
			int compute_up_range = min(boky, radius);
			int compute_down_range = min(work_height - 1 - boky, radius);
			int compute_left_range = min(bokx, radius);
			int compute_right_range = min(work_width - 1 - bokx, radius);
			for(int kr = -compute_up_range; kr <= compute_down_range; kr+=2)
				{
					int offset = (boky + kr) * work_width + bokx;
					for(int kc = -compute_left_range; kc <= compute_right_range; kc+=2)
						{	
							if( abs(src_buffer[dst_pixel_id] - src_buffer[offset + kc]) < 10 )
								{
									sumC0 +=  (int)src_buffer[offset + kc];
									sumWeight++;
								}
						}
				}
			if(sumWeight != 0)
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





CLRuntime::CLRuntime()
{
	long t1, t2;

	cl_int status = 0;
    size_t deviceListSize;
 
    // 获取opencl可用平台，当前为ARM
	cl_uint numPlatforms;
	
	status = clGetPlatformIDs( 0, NULL, &numPlatforms);
	if(status != CL_SUCCESS){
	    printf("Error: Getting Platforms\n");
	    //return EXIT_FAILURE;
	}

	if (numPlatforms > 0) {
	    cl_platform_id *platforms = (cl_platform_id *)malloc(numPlatforms * sizeof(cl_platform_id));
	    status = clGetPlatformIDs(numPlatforms, platforms, NULL);
	    if (status != CL_SUCCESS) {
	        printf("Error: Getting Platform Ids.(clGetPlatformIDs)\n");
	        //return EXIT_FAILURE;
	    }
		
		platform = platforms[0];
		free(platforms);
	}

	// 如果我们能找到相应平台，就使用它，否则返回NULL
    cl_context_properties cps[3] = {
        CL_CONTEXT_PLATFORM,
        (cl_context_properties)platform,
        0
    };
 
    cl_context_properties *cprops = (NULL == platform) ? NULL : cps;
 
 
    // 生成 context
    gpu_context = clCreateContextFromType(
                             cprops,
                             CL_DEVICE_TYPE_GPU,
                             NULL,
                             NULL,
                             &status);
    if (status != CL_SUCCESS) {
        printf("Error: Creating Context.(clCreateContexFromType)\n");
        //return EXIT_FAILURE;
    }

	// 寻找OpenCL设备
 
    // 首先得到设备列表的长度
    status = clGetContextInfo(gpu_context,
                              CL_CONTEXT_DEVICES,
                              0,
                              NULL,
                              &deviceListSize);
    if (status != CL_SUCCESS) {
        printf("Error: Getting Context Info device list size, clGetContextInfo)\n");
        //return EXIT_FAILURE;
    }

    devices = (cl_device_id *)malloc( sizeof(cl_device_id) * deviceListSize);
    if (devices == NULL) {
        printf("Error: No devices found.\n");
       //return EXIT_FAILURE;
    }
 
    // 现在得到设备列表
    status = clGetContextInfo(gpu_context,
                              CL_CONTEXT_DEVICES,
                              deviceListSize,
                              devices,
                              NULL);
    if (status != CL_SUCCESS) {
        printf("Error: Getting Context Info (device list, clGetContextInfo)\n");
        //return EXIT_FAILURE;
    }

	// 装载内核程序，编译CL program ,生成CL内核实例
    
	size_t sourceSize_gradsmooth[] = {strlen(kernelSourceCode_grad_smootch)};
    cl_program program_gradsmooth = clCreateProgramWithSource(gpu_context,
                         1,
                         &kernelSourceCode_grad_smootch,
                         sourceSize_gradsmooth,
                         &status);
	
	
    if (status != CL_SUCCESS) {
        printf("Error: Loading Binary into cl_program (clCreateProgramWithBinary)\n");
        //return EXIT_FAILURE;
    }
 
    // 为指定的设备编译CL program.
    const char options[] = "-cl-single-precision-constant -cl-fast-relaxed-math";
   


	//gradsmooth
	status = clBuildProgram(program_gradsmooth, 1, devices, options, NULL, NULL);
    if (status != CL_SUCCESS) {
        printf("Error: Building program_gradsmooth (clBuildingProgram) err_code:%d \n", status);
        //return EXIT_FAILURE;
    }

#if DEBUG_PRINT
	//print some info
	printf("num of GPU device find from context:%d\n", deviceListSize);
	//查询设备的名称
	char buffer[110];
	status = clGetDeviceInfo(devices[0], CL_DEVICE_NAME, 100, buffer, NULL);
	printf("===Device Name:%s===\n", buffer);
	//查询设备类型
	cl_device_type dev_type;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_TYPE, sizeof(cl_device_type), &dev_type, NULL);
	printf("Device type: %lu\n", dev_type);
	//查询设备计算单元最大数目
	cl_uint UnitNum;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_COMPUTE_UNITS, sizeof(cl_uint), &UnitNum, NULL);
	printf("Compute Units Number: %d\n", UnitNum);
	//查询设备核心频率
	cl_uint frequency;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_CLOCK_FREQUENCY, sizeof(cl_uint), &frequency, NULL);
	printf("Device Frequency: %d(MHz)\n", frequency);
	//查询设备全局内存大小
	cl_ulong GlobalSize;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_GLOBAL_MEM_SIZE, sizeof(cl_ulong), &GlobalSize, NULL);
	printf("Device Global Mem Size: %0.0f(MB)\n", (float)GlobalSize/1024/1024);
	//查询设备全局内存缓存行
	cl_uint GlobalCacheLine;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_GLOBAL_MEM_CACHELINE_SIZE, sizeof(cl_uint), &GlobalCacheLine, NULL);
	printf("Device Global Mem CacheLine: %d(Byte)\n", GlobalCacheLine);
	//查询设备最大工作组
	cl_ulong GlobalWGSize;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_GROUP_SIZE, sizeof(cl_ulong), &GlobalWGSize, NULL);
	printf("Device Global MAX_WORK_GROUP Size: %lu\n", GlobalWGSize);
	//查询设备最大工作组
	cl_ulong GlobalWISize;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(cl_ulong), &GlobalWISize, NULL);
	printf("Device Global MAX_WORK_ITEM Size: %lu\n", GlobalWISize);

	cl_ulong LOCAL_MEM_SIZE;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_LOCAL_MEM_SIZE, sizeof(cl_ulong), &LOCAL_MEM_SIZE, NULL);
	printf("Device Global LOCAL_MEM_SIZE Size: %0.0f(KB)\n", (float)LOCAL_MEM_SIZE/1024);
	
	//查询设备支持的OpenCL版本
	char DeviceVersion[110];
	status = clGetDeviceInfo(devices[0], CL_DEVICE_VERSION, 100, DeviceVersion, NULL);
	printf("Device Version:%s\n", DeviceVersion);
	//查询设备扩展名
	char* DeviceExtensions;
	size_t ExtenNum;
	status = clGetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, 0, NULL, &ExtenNum);
	DeviceExtensions = (char*)malloc(ExtenNum);
	status = clGetDeviceInfo(devices[0], CL_DEVICE_EXTENSIONS, ExtenNum, DeviceExtensions, NULL);
	printf("Device Extensions: %s\n", DeviceExtensions);
#endif	
 
    // 得到指定名字的内核实例的句柄
	kernel_gradsmooth = clCreateKernel(program_gradsmooth, "grad_smootch", &status);
    if (status != CL_SUCCESS) {
        printf("Error: Creating kernel_gradsmooth from program.(clCreateKernel)\n");
        //return EXIT_FAILURE;
    }

	status = clReleaseProgram(program_gradsmooth);

}

CLRuntime::~CLRuntime()
{
	
	clReleaseKernel(kernel_gradsmooth);

	//release context
    clReleaseContext(gpu_context);
	free(devices);
	
#if DEBUG_PRINT
	printf("***CLRuntime resource released! ***\n");
#endif
}



int cL_gradsmooth(Mat & depth_img, Mat & grad, 
						 const int& thresh, const int & Radius, CLRuntime &bok_cl_runtime )
{
	cl_int status = 0;
#if DEBUG_TIME
	long t1,t2;
	t1 = getCPUTickCount();
#endif

	const unsigned long src_width = depth_img.cols;
	const unsigned long src_height = depth_img.rows;
	//cout<<"srcImg2 size: "<<srcImg_2.cols <<" * "<<srcImg_2.rows<<endl;


	unsigned char* grad_ptr = grad.data;
	unsigned char* src_ptr = depth_img.data;
	Mat dst_img = Mat(depth_img.size(), CV_8UC1 );
	//Mat blurredImg_2 = srcImg_2.clone();
	unsigned char* dst = dst_img.data;
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
		printf("Error: Create Buffer, outputBuffer. (clCreateBuffer)\n");
	}
 
	//t1 = getCPUTickCount();
	unsigned int arg_id = 0;
	status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_mem), (void *)&src_buffer);
	status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_mem), (void *)&grad_buffer);
	status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_mem), (void *)&dst_buffer);
	status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_int), (void *)&thresh);
	status = clSetKernelArg(bok_cl_runtime.kernel_gradsmooth, arg_id++, sizeof(cl_int), (void *)&Radius);
	//t2 = getCPUTickCount();
	//cout << "clSetKernelArg takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
	
	//t1 = getCPUTickCount();
	cl_command_queue commandQueue = clCreateCommandQueue(bok_cl_runtime.gpu_context,
									bok_cl_runtime.devices[0],
									0,
									&status);
	//t2 = getCPUTickCount();
	//cout << "clCreateCommandQueue takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
	if (status != CL_SUCCESS) {
		printf("Error: Create Command Queue. (clCreateCommandQueue)\n");
	}

#if DEBUG_TIME
	t1 = getCPUTickCount();
#endif
	cl_uint work_dim = 2;
	size_t globalThreads[] = {src_width , src_height};
	//size_t localThreads[] = {32, 12};
	
	status = clEnqueueNDRangeKernel(commandQueue, bok_cl_runtime.kernel_gradsmooth,
									work_dim, NULL, globalThreads,
									NULL, 0,
									NULL, NULL);
	if (status != CL_SUCCESS) {
		printf("Error: Enqueueing kernel err_code:%d \n",status);
	}
 
	status = clFinish(commandQueue);
	if (status != CL_SUCCESS) {
		printf("Error: Finish command queue\n");
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
								 sizeof(unsigned char) * src_width * src_height, dst, 0, NULL, NULL);
#if DEBUG_TIME
	t2 = getCPUTickCount();
	cout << "clEnqueueReadBuffer takes: " << (t2 - t1) * 1000 / getTickFrequency() << endl;
#endif

	if (status != CL_SUCCESS) {
		printf("Error: Read buffer queue err_code:%d \n",status);
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


