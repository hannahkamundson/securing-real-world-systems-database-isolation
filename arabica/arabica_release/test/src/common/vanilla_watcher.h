#include <jni.h>
#include <sys/time.h>

//measurement instrumentation
jclass robusta_watcher;
jmethodID update_sand;

typedef struct timeval * TIMEVAL_PTR;

void updateTime(JNIEnv * env, TIMEVAL_PTR before, TIMEVAL_PTR after);
