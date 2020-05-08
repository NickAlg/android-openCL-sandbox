
#include <jni.h>
#include <string>
#include "CLbackend.h"
#include "android/log.h"
#include "android_log.h"


extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_administrator_demo_1ndk_1opencv_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    CLRuntime *tmp = new CLRuntime();
    std::string hello;
//    for (int i = 0; i <tmp->deviceListSize ; ++i) {
    int itmp = tmp->init();
  LOG_W2(ANDROID_LOG_DEBUG, "%d = tmp->init() ", itmp);
  LOG(ANDROID_LOG_DEBUG, "%d = tmp->init() ", itmp);
    if (itmp == CL_SUCCESS) {

        // Load kernel program, compile CL program, generate CL kernel instance
        hello.append(tmp->getInfo(0));
        LOGW( "%s", hello.c_str());
        tmp->getInfoSt2(0);

    } else hello = std::string(" No Cl found in this system");
    delete tmp;
    return env->NewStringUTF(hello.c_str());
}
