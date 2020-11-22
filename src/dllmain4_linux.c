#include <stdio.h>
#include <time.h>

#include "redismodule.h"
#include "hydrology.h"
#include "module.h"

int main() {
	return 0;
}

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

	if (RedisModule_CreateCommand(ctx, "raintype.insert",
		redis_insert, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.search",
		redis_search, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.sum",
		redis_sum, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.now",
		redis_sum, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.time",
		redis_time_range, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	return REDISMODULE_OK;
}


