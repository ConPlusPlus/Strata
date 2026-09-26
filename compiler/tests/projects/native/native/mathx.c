#include "mathx.h"
#ifdef _WIN32
#include <windows.h>
#include <mmsystem.h>
int mathx_clock_ok(void) { return timeGetTime() > 0; }
#else
int mathx_clock_ok(void) { return 1; }
#endif
int mathx_scale(int x) { return x * MATHX_SCALE; }
