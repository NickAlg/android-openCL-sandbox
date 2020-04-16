#include <jni.h>
#include <string>
#include "CLbackend.h"
#include "android/log.h"

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_administrator_demo_1ndk_1opencv_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    CLRuntime *tmp = new CLRuntime();
    std::string hello;
//    for (int i = 0; i <tmp->deviceListSize ; ++i) {
    int itmp = tmp->init();

    __android_log_print(ANDROID_LOG_WARN, " TODEL ", "%d = tmp->init() ", itmp);
    if (itmp == CL_SUCCESS) {

        // Load kernel program, compile CL program, generate CL kernel instance
        hello.append(tmp->getInfo(0));
        __android_log_print(ANDROID_LOG_WARN, " TODEL ", "%s", hello.c_str());
        tmp->getInfoSt2(0);

    }
    delete tmp;
    return env->NewStringUTF(hello.c_str());
}
