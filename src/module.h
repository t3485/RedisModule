#ifndef  MODULE_H
#define MODULE_H 

#ifdef _WINDOWS
#include "win32_types_hiredis.h"
#endif
#include "hydrology.h"

#include "redismodule.h"
#include <time.h>

int redis_search(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int redis_sum(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
int redis_insert(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
void redis_save(RedisModuleIO* rdb, void* value);
void* redis_load(RedisModuleIO* rdb, int encver);
void redis_aof(RedisModuleIO* aof, RedisModuleString* key, void* value);
int redis_time_range(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);
RainObject* redis_create();
void redis_free(RainObject* o);
size_t redis_usage(const void* value);
int redis_max(RedisModuleCtx* ctx, RedisModuleString** argv, int argc);

extern struct RedisModuleType* g_raintype;

#endif