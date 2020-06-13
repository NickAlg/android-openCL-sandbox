//
// Created by nickzt on 07.05.20.
//

#ifndef ANDROID_NDK_DEMO_APP_SRC_MAIN_CPP_ANDROID_LOG_H_
#define ANDROID_NDK_DEMO_APP_SRC_MAIN_CPP_ANDROID_LOG_H_

#include <android/log.h>

#define LOG_TAG " TODEL "

# define LOG_W2(LEVEL , fmt ,...) \
__android_log_print ( LEVEL , LOG_TAG , "%s:%i : %s" fmt , __FILE__ , __LINE__ ,\
__PRETTY_FUNCTION__ , ## __VA_ARGS__ )

#define LOGD(fmt ,...) \
__android_log_print ( ANDROID_LOG_DEBUG , LOG_TAG , "%s:%i : %s" fmt , __FILE__ , __LINE__ ,\
__PRETTY_FUNCTION__ , ## __VA_ARGS__ )

#define LOGW(fmt ,...) \
__android_log_print ( ANDROID_LOG_WARN , LOG_TAG , "%s:%i : %s" fmt , __FILE__ , __LINE__ ,\
__PRETTY_FUNCTION__ , ## __VA_ARGS__ )

#define LOGE(fmt ,...) \
__android_log_print ( ANDROID_LOG_ERROR , LOG_TAG , "%s:%i : %s" fmt , __FILE__ , __LINE__ ,\
__PRETTY_FUNCTION__ , ## __VA_ARGS__ )
#endif //ANDROID_NDK_DEMO_APP_SRC_MAIN_CPP_ANDROID_LOG_H_
