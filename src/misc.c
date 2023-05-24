// misc.c

#ifdef _WIN64
#include "windows.h"
#else
#include "sys/time.h"
#endif

int get_time_ms(void)
{
#ifdef _WIN64
	return GetTickCount64();
#else
  struct timeval t;
  gettimeofday(&t, NULL);
  return t.tv_sec*1000 + t.tv_usec/1000;
#endif
}
