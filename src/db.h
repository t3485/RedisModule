#ifndef DB_H
#define DB_H 

#include "common.h"

struct _RedisDb {
	float last;
	unsigned long long count, ver;
	RedisModuleIO* rdb;

	void (*save)(struct _RedisDb* p, float v);
	void (*flush)(struct _RedisDb* p);
	float (*load)(struct _RedisDb* p);
};

typedef struct _RedisDb RedisDb;

RedisDb* db_init(RedisDb* p, RedisModuleIO* rdb, int ver);

#endif