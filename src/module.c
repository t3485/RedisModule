#include "module.h"

RedisModuleType* g_raintype;

RainObject* create() {
	RainObject* o = RedisModule_Alloc(sizeof(*o));
	hyd_init(o);
	return o;
}

void obj_free(RainObject* o) {
	RedisModule_Free(o);
}

int redis_search(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx);

	if (argc != 4) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != g_raintype) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long begin, end;
	if (RedisModule_StringToLongLong(argv[2], &begin) != REDISMODULE_OK ||
		RedisModule_StringToLongLong(argv[3], &end) != REDISMODULE_OK ||
		begin < 0 || end < 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a number");
	}

	RainObject* o = RedisModule_ModuleTypeGetValue(key);
	if (o) {
		struct SearchResult r;
		long long b, e;
		double value;
		hyd_search(o, begin, end, &r);
		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		b = 0;
		while (b++ < r.pre)
			RedisModule_ReplyWithDouble(ctx, g_null);

		b = r.index;
		e = r.index + r.size;
		while (b < e) {
			if (b >= g_data_length)
				value = o->data[b - g_data_length];
			else value = o->data[b];
			b++;
			RedisModule_ReplyWithDouble(ctx, value);
		}
		b = 0;
		while (b++ < r.suf)
			RedisModule_ReplyWithDouble(ctx, g_null);

		RedisModule_ReplySetArrayLength(ctx, r.pre + r.size + r.suf);
	}
	else {
		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		RedisModule_ReplySetArrayLength(ctx, 0);
	}
	return REDISMODULE_OK;
}

int redis_sum(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 4) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != g_raintype) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long begin, end;
	if (RedisModule_StringToLongLong(argv[2], &begin) != REDISMODULE_OK ||
		RedisModule_StringToLongLong(argv[3], &end) != REDISMODULE_OK ||
		begin < 0 || end < 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a number");
	}

	RainObject* o = RedisModule_ModuleTypeGetValue(key);
	if (o)
		RedisModule_ReplyWithDouble(ctx, hyd_sum(o, begin, end));
	else RedisModule_ReplyWithDouble(ctx, 0);

	return REDISMODULE_OK;
}

int redis_max(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 4) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != g_raintype) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long begin, end;
	if (RedisModule_StringToLongLong(argv[2], &begin) != REDISMODULE_OK ||
		RedisModule_StringToLongLong(argv[3], &end) != REDISMODULE_OK ||
		begin < 0 || end < 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a number");
	}

	if (begin > end) {
		return RedisModule_ReplyWithError(ctx, "end must greater than or equal begin");
	}

	RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
	RainObject* o = RedisModule_ModuleTypeGetValue(key);
	if (o) {
		struct SearchResult r;
		float max = hyd_max(o, begin, end, &r);
		RedisModule_ReplyWithDouble(ctx, max);
		RedisModule_ReplyWithDouble(ctx, r.index);
		RedisModule_ReplySetArrayLength(ctx, 2);
	}
	else {
		RedisModule_ReplySetArrayLength(ctx, 0);
	}

	return REDISMODULE_OK;
}

int redis_insert(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc <= 3 && argc > g_max_count + 3) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != g_raintype) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long time;
	if ((RedisModule_StringToLongLong(argv[2], &time) != REDISMODULE_OK)) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a double");
	}

	if (time <= 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: time can not be zero or negative");
	}

	double value[g_max_count];
	for (size_t i = 3; i < argc; i++) {
		if ((RedisModule_StringToDouble(argv[i], &value[i - 3]) != REDISMODULE_OK)) {
			return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a double");
		}
	}

	RainObject* o;
	if (type == REDISMODULE_KEYTYPE_EMPTY) {
		o = redis_create();
		RedisModule_ModuleTypeSetValue(key, g_raintype, o);
	}
	else o = RedisModule_ModuleTypeGetValue(key);

	hyd_insert(o, value, time, (long long)argc - 3);

	RedisModule_ReplyWithLongLong(ctx, hyd_len(o));
	RedisModule_ReplicateVerbatim(ctx);
	return REDISMODULE_OK;
}

void redis_save(RedisModuleIO* rdb, void* value) {
	RainObject* hto = value;
	RedisModule_SaveSigned(rdb, hto->time);
	RedisModule_SaveSigned(rdb, hto->end);
	RedisModule_SaveSigned(rdb, hto->full ? g_data_length : hto->end + 1);

	long long i = hto->full ? hto->end + 1 : 0,
		e = hto->full ? g_data_length - 1 : hto->end;
	i = checkIndex(hto, i);

	while (i <= e) {
		RedisModule_SaveFloat(rdb, hto->data[i++]);
	}

	if (hto->full) {
		i = 0;
		while (i <= hto->end) {
			RedisModule_SaveFloat(rdb, hto->data[i++]);
		}
	}
}

void* redis_load_ver_0(RedisModuleIO* rdb) {
	long long time = RedisModule_LoadUnsigned(rdb),
		end = RedisModule_LoadUnsigned(rdb),
		size = RedisModule_LoadUnsigned(rdb),
		i = 0;

	RainObject* hto = redis_create();
	hto->end = end;
	hto->time = time;
	hto->full = size >= g_data_length;

	if (hto->full) {
		i = size - g_data_length;
		while (--i >= 0)
			RedisModule_LoadDouble(rdb);
		size = g_data_length;
		i = 0;
	}

	while (i < size)
		hto->data[i++] = (float)RedisModule_LoadDouble(rdb);
	return hto;
}

void* redis_load_ver_1(RedisModuleIO* rdb) {
	long long time = RedisModule_LoadUnsigned(rdb),
		end = RedisModule_LoadUnsigned(rdb),
		size = RedisModule_LoadUnsigned(rdb),
		i = 0;

	RainObject* hto = redis_create();
	hto->end = end;
	hto->time = time;
	hto->full = size >= g_data_length;

	if (hto->full) {
		i = size - g_data_length;
		while (--i >= 0)
			RedisModule_LoadFloat(rdb);
		size = g_data_length;
		i = 0;
	}

	while (i < size)
		hto->data[i++] = (float)RedisModule_LoadFloat(rdb);
	return hto;
}

void* redis_load(RedisModuleIO* rdb, int encver) {
	if (encver == 0)
	{
		redis_load_ver_0(rdb);
	}
	else
	{
		redis_load_ver_1(rdb);
	}
}

char* float2str(double d, char* str)
{
	char str1[128];
	int j = 0, k, i;
	i = (int)d;//浮点数的整数部分
	while (i > 0)
	{
		str1[j++] = i % 10 + '0';
		i = i / 10;
	}
	for (k = 0; k < j; k++)
	{
		str[k] = str1[j - 1 - k];//被提取的整数部分正序存放到另一个数组
	}
	str[j++] = '.';

	d = d - (int)d;//小数部分提取
	for (i = 0; i < 10; i++)
	{
		d = d * 10;
		str[j++] = (int)d + '0';
		d = d - (int)d;
	}
	while (str[--j] == '0');
	str[++j] = '\0';
	return str;
}

void redis_aof(RedisModuleIO* aof, RedisModuleString* key, void* value) {
	RainObject* o = value;
	char data[40];

	long long i = o->full ? o->end + 1 : 0,
		e = o->full ? g_data_length - 1 : o->end,
		time = o->time - o->end;
	i = checkIndex(o, i);

	while (i <= e) {
		float2str(o->data[i++], data);
		RedisModule_EmitAOF(aof, "hydinsert", "slc", key, time++, data);
	}

	if (o->full) {
		i = 0;
		while (i <= o->end) {
			float2str(o->data[i++], data);
			RedisModule_EmitAOF(aof, "hydinsert", "slc", key, time++, data);
		}
	}
}

void time2str(long long n, char* buf, size_t len)
{
	time_t time = n;
	struct tm p;
	time_t min = 1600000000, max = 2000000000;

	time *= 5 * 60;
	if (time > max)
	{
		buf[0] = '\0';
		return;
	}
	else if (time < min)
	{
		time *= 12;
		if (time > max || time < min)
		{
			buf[0] = '\0';
			return;
		}
	}

#ifdef _WINDOWS
	localtime_s(&p, &time);
#else
	localtime_r(&time, &p);
#endif
	strftime(buf, len, "%Y-%m-%d %H:%M:%S", &p);
}

int redis_time_range(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 2) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != g_raintype) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	RainObject* hto = RedisModule_ModuleTypeGetValue(key);
	if (hto != NULL)
	{
		long long begin = hto->full ? hto->time - g_data_length : hto->time - hto->end;

		char* buf = RedisModule_Alloc(26);
		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		RedisModule_ReplyWithLongLong(ctx, begin);
		time2str(begin, buf, 26);
		RedisModule_ReplyWithCString(ctx, buf);
		RedisModule_ReplyWithLongLong(ctx, hto->time);
		time2str(hto->time, buf, 26);
		RedisModule_ReplyWithCString(ctx, buf);
		RedisModule_Free(buf);
		RedisModule_ReplySetArrayLength(ctx, 4);
	}
	else
		RedisModule_ReplyWithNull(ctx);

	return REDISMODULE_OK;
}

RainObject* redis_create() {
	RainObject* o = RedisModule_Alloc(sizeof(*o));
	hyd_init(o);
	return o;
}

void redis_free(RainObject* o) {
	RedisModule_Free(o);
}

size_t redis_usage(const void* value) {
	const RainObject* o = value;
	return sizeof(*o);
}
