#include <stdio.h>
#include <time.h>

#ifdef _WINDOWS
#include <windows.h>
#include "win32_types_hiredis.h"
#endif

#include "redismodule.h"
#include "hydrology.h"
#include "module.h"

#ifdef _WINDOWS

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}
#else
int main() {
	return 0;
}
#endif

#ifdef _WINDOWS
__declspec(dllexport)
#endif
int RedisModule_OnLoad(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	REDISMODULE_NOT_USED(argv);
	REDISMODULE_NOT_USED(argc);

	if (RedisModule_Init(ctx, "raintype", 1, REDISMODULE_APIVER_1)
		== REDISMODULE_ERR) return REDISMODULE_ERR;

	RedisModuleTypeMethods tm = {
		.version = REDISMODULE_TYPE_METHOD_VERSION,
		.rdb_load = redis_load,
		.rdb_save = redis_save,
		.aof_rewrite = redis_aof,
		.mem_usage = redis_usage,
		.free = redis_free,
		.digest = NULL
	};

	g_raintype = RedisModule_CreateDataType(ctx, "rain-type", 0, &tm);
	if (g_raintype == NULL) return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "hydinsert",
		redis_insert, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "hydrange",
		redis_search, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "hydsum",
		redis_sum, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "hydtime",
		redis_time_range, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	return REDISMODULE_OK;
}