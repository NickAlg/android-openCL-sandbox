//
// Created by nickzt on 07.05.20.
//

#ifndef ANDROID_NDK_DEMO_APP_SRC_MAIN_CPP_ANDROID_LOG_H_
#define ANDROID_NDK_DEMO_APP_SRC_MAIN_CPP_ANDROID_LOG_H_

#include <android/log.h>

#define LOG_TAG "OPENCLTST "

#define LOG(LOGT,x...) do { \
  char buf[512]; \
  sprintf(buf, x); \
  __android_log_print(LOGT,LOG_TAG, "%s:%i| %s",  __FILE__, __LINE__,  buf); \
} while (0)

# define LOG_W2 ( level , fmt ,...) \
__android_log_print ( level , LOG_TAG , " (% s :% u ) %s: " fmt , \
__FILE__ , __LINE__ , __PRETTY_FUNCTION__ , ## __VA_ARGS__ )

#define LOGD(x...) do { \
  char buf[512]; \
  sprintf(buf, x); \
  __android_log_print(ANDROID_LOG_DEBUG,LOG_TAG, "%s:%i| %s",  __FILE__, __LINE__,  buf); \
} while (0)
#define LOGW(x...) do { \
  char buf[512]; \
  sprintf(buf, x); \
  __android_log_print(ANDROID_LOG_WARN,LOG_TAG, "%s:%i| %s",  __FILE__, __LINE__,  buf); \
} while (0)
#define LOGE(x...) do { \
  char buf[512]; \
  sprintf(buf, x); \
  __android_log_print(ANDROID_LOG_ERROR,LOG_TAG, "%s:%i| %s",  __FILE__, __LINE__,  buf); \
} while (0)
#endif //ANDROID_NDK_DEMO_APP_SRC_MAIN_CPP_ANDROID_LOG_H_
