# For more information about using CMake with Android Studio, read the
# documentation: https://d.android.com/studio/projects/add-native-code.html

# Sets the minimum version of CMake required to build the native library.

cmake_minimum_required(VERSION 3.4.1)

# opencv configuration
set( OpenCV_ANDROID_SDK E:/opencv-4.1.1-android-sdk-omp )
set( OpenCV_DIR ${OpenCV_ANDROID_SDK}/sdk/native/jni )

find_package(OpenCV REQUIRED core highgui calib3d imgproc imgcodecs features2d ximgproc photo)
if(OpenCV_FOUND)
    include_directories(${OpenCV_INCLUDE_DIRS})
    message(STATUS "OpenCV library status:")
    message(STATUS "    version: ${OpenCV_VERSION}")
    message(STATUS "    libraries: ${OpenCV_LIBS}")
    message(STATUS "    include path: ${OpenCV_INCLUDE_DIRS}")
else(OpenCV_FOUND)
    message(FATAL_ERROR "OpenCV library not found")
endif(OpenCV_FOUND)

# Creates and names a library, sets it as either STATIC
# or SHARED, and provides the relative paths to its source code.
# You can define multiple libraries, and CMake builds them for you.
# Gradle automatically packages shared libraries with your APK.

add_library( # Sets the name of the library.
             native-lib

             # Sets the library as a shared library.
             SHARED

             # Provides a relative path to your source file(s).
             src/main/cpp/native-lib.cpp
             src/main/cpp/CLbackend.cpp)


include_directories(src/main/cpp/)
add_library(OpenCL STATIC src/main/cpp/libopencl.cpp)
if(CMAKE_CXX_COMPILER_ID STREQUAL "Clang" OR
    CMAKE_CXX_COMPILER_ID STREQUAL "GNU"   OR
    CMAKE_CXX_COMPILER_ID STREQUAL "AppleClang")
    target_compile_options(OpenCL PRIVATE -O2 -fPIC -Wall)
endif()


# Searches for a specified prebuilt library and stores the path as a
# variable. Because CMake includes system libraries in the search path by
# default, you only need to specify the name of the public NDK library
# you want to add. CMake verifies that the library exists before
# completing its build.

find_library( # Sets the name of the path variable.
              log-lib

              # Specifies the name of the NDK library that
              # you want CMake to locate.
              log )

# Specifies libraries CMake should link to your target library. You
# can link multiple libraries, such as libraries you define in this
# build script, prebuilt third-party libraries, or system libraries.

target_link_libraries( # Specifies the target library.
                       native-lib
                       ${OpenCV_LIBRARIES}
                       OpenCL
                       # Links the target library to the log library
                       # included in the NDK.
                       ${log-lib} )

# add open mp for opencv
SET ( CMAKE_CXX_FLAGS "-fopenmp ${CMAKE_CXX_FLAGS}" )