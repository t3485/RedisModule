#ifndef _COMMON_H
#define _COMMON_H 

#ifdef _WINDOWS
#include <windows.h>
#include "win32_types_hiredis.h"
#include "redismodule.h"
#else
#include "linux_redismodule.h"
#endif

#include <float.h>

extern const float g_split;

/*
* ģ��汾��
*/
extern const int g_ver;

#endif