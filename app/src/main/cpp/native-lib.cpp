#include <jni.h>
#include <string>

extern "C" JNIEXPORT jstring

JNICALL
Java_com_example_administrator_demo_1ndk_1opencv_MainActivity_stringFromJNI(
        JNIEnv *env,
        jobject /* this */) {
    std::string hello = "弘樹です、よろしくお願いします！";
    return env->NewStringUTF(hello.c_str());
}
