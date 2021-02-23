#ifndef _COMMON_H
#define _COMMON_H 

#include <float.h>
#ifdef _WINDOWS
#include <windows.h>
#include "win32_types_hiredis.h"
#endif

#include "redismodule.h"

extern const float g_split;
extern const float g_null;

/*
* Ä£¿é°æ±¾ºÅ
*/
extern const int g_ver;

enum {
	g_max_count = 288,
	g_data_length = 288 * 92
};
#endif