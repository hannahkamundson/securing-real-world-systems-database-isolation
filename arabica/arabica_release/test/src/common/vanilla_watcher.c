#include "vanilla_watcher.h"

void updateTime(JNIEnv * env, TIMEVAL_PTR before, TIMEVAL_PTR after)
{
	long int sand=0;

	sand = 1000000*(after->tv_sec-before->tv_sec)+(after->tv_usec-before->tv_usec);

	if(robusta_watcher != NULL)
	{
		(*env)->CallStaticVoidMethod(env, robusta_watcher, update_sand, sand);
	}
}
