#ifndef __CL_BACKEND_H
#define __CL_BACKEND_H

#include <string>
#include <time.h>
#include "opencv2/opencv.hpp"
#include <string.h>

#include "OpenCLWrapper.h"

using namespace std;
using namespace cv;


#define USE_PAD2 0
#define USE_WORKGROUP 0

static const char *getErrorString(cl_int error) {
    switch (error) {
// run-time and JIT compiler errors
        case 0:
            return "CL_SUCCESS";
        case -1:
            return "CL_DEVICE_NOT_FOUND";
        case -2:
            return "CL_DEVICE_NOT_AVAILABLE";
        case -3:
            return "CL_COMPILER_NOT_AVAILABLE";
        case -4:
            return "CL_MEM_OBJECT_ALLOCATION_FAILURE";
        case -5:
            return "CL_OUT_OF_RESOURCES";
        case -6:
            return "CL_OUT_OF_HOST_MEMORY";
        case -7:
            return "CL_PROFILING_INFO_NOT_AVAILABLE";
        case -8:
            return "CL_MEM_COPY_OVERLAP";
        case -9:
            return "CL_IMAGE_FORMAT_MISMATCH";
        case -10:
            return "CL_IMAGE_FORMAT_NOT_SUPPORTED";
        case -11:
            return "CL_BUILD_PROGRAM_FAILURE";
        case -12:
            return "CL_MAP_FAILURE";
        case -13:
            return "CL_MISALIGNED_SUB_BUFFER_OFFSET";
        case -14:
            return "CL_EXEC_STATUS_ERROR_FOR_EVENTS_IN_WAIT_LIST";
        case -15:
            return "CL_COMPILE_PROGRAM_FAILURE";
        case -16:
            return "CL_LINKER_NOT_AVAILABLE";
        case -17:
            return "CL_LINK_PROGRAM_FAILURE";
        case -18:
            return "CL_DEVICE_PARTITION_FAILED";
        case -19:
            return "CL_KERNEL_ARG_INFO_NOT_AVAILABLE";

// compile-time errors
        case -30:
            return "CL_INVALID_VALUE";
        case -31:
            return "CL_INVALID_DEVICE_TYPE";
        case -32:
            return "CL_INVALID_PLATFORM";
        case -33:
            return "CL_INVALID_DEVICE";
        case -34:
            return "CL_INVALID_CONTEXT";
        case -35:
            return "CL_INVALID_QUEUE_PROPERTIES";
        case -36:
            return "CL_INVALID_COMMAND_QUEUE";
        case -37:
            return "CL_INVALID_HOST_PTR";
        case -38:
            return "CL_INVALID_MEM_OBJECT";
        case -39:
            return "CL_INVALID_IMAGE_FORMAT_DESCRIPTOR";
        case -40:
            return "CL_INVALID_IMAGE_SIZE";
        case -41:
            return "CL_INVALID_SAMPLER";
        case -42:
            return "CL_INVALID_BINARY";
        case -43:
            return "CL_INVALID_BUILD_OPTIONS";
        case -44:
            return "CL_INVALID_PROGRAM";
        case -45:
            return "CL_INVALID_PROGRAM_EXECUTABLE";
        case -46:
            return "CL_INVALID_KERNEL_NAME";
        case -47:
            return "CL_INVALID_KERNEL_DEFINITION";
        case -48:
            return "CL_INVALID_KERNEL";
        case -49:
            return "CL_INVALID_ARG_INDEX";
        case -50:
            return "CL_INVALID_ARG_VALUE";
        case -51:
            return "CL_INVALID_ARG_SIZE";
        case -52:
            return "CL_INVALID_KERNEL_ARGS";
        case -53:
            return "CL_INVALID_WORK_DIMENSION";
        case -54:
            return "CL_INVALID_WORK_GROUP_SIZE";
        case -55:
            return "CL_INVALID_WORK_ITEM_SIZE";
        case -56:
            return "CL_INVALID_GLOBAL_OFFSET";
        case -57:
            return "CL_INVALID_EVENT_WAIT_LIST";
        case -58:
            return "CL_INVALID_EVENT";
        case -59:
            return "CL_INVALID_OPERATION";
        case -60:
            return "CL_INVALID_GL_OBJECT";
        case -61:
            return "CL_INVALID_BUFFER_SIZE";
        case -62:
            return "CL_INVALID_MIP_LEVEL";
        case -63:
            return "CL_INVALID_GLOBAL_WORK_SIZE";
        case -64:
            return "CL_INVALID_PROPERTY";
        case -65:
            return "CL_INVALID_IMAGE_DESCRIPTOR";
        case -66:
            return "CL_INVALID_COMPILER_OPTIONS";
        case -67:
            return "CL_INVALID_LINKER_OPTIONS";
        case -68:
            return "CL_INVALID_DEVICE_PARTITION_COUNT";

// extension errors
        case -1000:
            return "CL_INVALID_GL_SHAREGROUP_REFERENCE_KHR";
        case -1001:
            return "CL_PLATFORM_NOT_FOUND_KHR";
        case -1002:
            return "CL_INVALID_D3D10_DEVICE_KHR";
        case -1003:
            return "CL_INVALID_D3D10_RESOURCE_KHR";
        case -1004:
            return "CL_D3D10_RESOURCE_ALREADY_ACQUIRED_KHR";
        case -1005:
            return "CL_D3D10_RESOURCE_NOT_ACQUIRED_KHR";
        default:
            return "Unknown OpenCL error";
    }
};

#define CHECK_CL_SUCCESS(err, x...)                  \
    if (err != CL_SUCCESS) {                       \
        LOG_W2(ANDROID_LOG_ERROR, "%s %d %s\n", x, err, getErrorString((err)) );\
    }

#define ASSERT_CL_SUCCESS(error, x...)                  \
    if (error != CL_SUCCESS) {                       \
        LOG_W2(ANDROID_LOG_ERROR, "Assertion CL_SUCCESS failed %s %d %s\n", x, err, getErrorString((err)) );\
        std::exit(err);\
    }

class CLRuntime {
public:
    CLRuntime();

    ~CLRuntime();

// Data members
    std::vector<cl_kernel> m_kernels;
    std::vector<cl_program> m_programs;


    cl_platform_id platformId = NULL;
    cl_device_id m_device = NULL;
    cl_context m_context = NULL;
    cl_command_queue m_cmd_queue = NULL;

    cl_uint retNumDevices;
    cl_uint retNumPlatforms;
    cl_program mProgram = NULL;
    cl_kernel mKErnelSobelFilter;


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

    class TimelineEntry {
    public:
        double start_time;
        double end_time;
        double execution_time;
        double api_overhead;
        double total_time;
        double cpu_time;
    };

    TimelineEntry AnalyzeEvent(cl_event &event);

    int iniKernelsgrandsmoothTest(cl_kernel &kernel_gradsmooth) const;

    std::stringstream getStringstream(
            char *info,
            cl_uint nPlatform,
            cl_platform_id *listPlatform,
            cl_uint &nDevice,
            cl_device_id *listDevice);

    std::string getCLDeviceName(int i, int j, cl_device_id device) const;

    /**
   * \brief Gets the cl_context associated with the wrapper for using in OpenCL functions.
   * @return
   */
    cl_device_id get_deviceID() { return m_device; }


    cl_command_queue make_queue(cl_int *err);

/**
 * \brief Makes a cl_kernel from the given program.
 *
 * @param kernel_name
 * @param program
 * @return
 */
    cl_kernel make_kernel(const std::string &kernel_name, cl_program program);

/**
 * \brief Gets the cl_context associated with the wrapper for using in OpenCL functions.
 * @return
 */
    cl_context get_context() const;

/**
* \brief Gets the cl_command_queue associated with the wrapper for using in OpenCL functions.
* @return
*/
    cl_command_queue get_command_queue() const;   //rename it to general queue
/**
 * Makes a cl_program (whose lifetime is managed by cl_sleeve) from the given source code strings.
 *
 * @param program_source - The source code strings.
 * @param program_source_len - The length of program_source
 * @return
 */
    cl_program make_program(const char **program_source, cl_uint program_source_len);

    std::string getInfo();

    std::string GetDevTypeSTR() const;

    std::string ObtainCLDeviceName(cl_device_id device) const;

    bool IsInited() const;

private:
    cl_int m_init_state_ = CL_DEVICE_NOT_FOUND;
    bool m_inited_ = false;
    cl_uint m_max_compute_units_ = 0;
    cl_device_type m_dev_type_ = CL_DEVICE_TYPE_CPU;
    size_t m_max_wrk_Size;
    std::string m_device_name;

};

int cL_gradsmooth(Mat &depth_img, Mat &grad,
                  const int &thresh, const int &Radius, CLRuntime &bok_cl_runtime);

#endif

