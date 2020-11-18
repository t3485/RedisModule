#include <stdio.h>
#include <time.h>
#include "redismodule.h"

static RedisModuleType* RainType;

int main() {
	return 0;
}
#define MAX_DAY_COUNT 288
#define MAX_DATA_LENGTH (MAX_DAY_COUNT * 92)
#define CheckIndex(i) if(i >= MAX_DATA_LENGTH){ i %= MAX_DATA_LENGTH; }

struct RainArrayObject {
	float data[MAX_DATA_LENGTH];
	long long end, time;
	char full, span;
};
typedef struct RainArrayObject RainObject;

RainObject* createRainArrayObject(void) {
	RainObject* o;
	o = RedisModule_Alloc(sizeof(*o));
	o->time = 0;
	o->end = -1;
	o->full = 0;
	return o;
}

void RainArrayReleaseObject(RainObject* o) {
	RedisModule_Free(o);
}

void TypeFree(void* value) {
	RainArrayReleaseObject(value);
}

size_t RainTypeMemUsage(const void* value) {
	const RainObject* o = value;
	return sizeof(*o);
}

void CopyData(RainObject* d, int index, double* data, long long l) {
	int i = 0;
	if (data) {
		while (i < l) {
			if (index == MAX_DATA_LENGTH)
				index = 0;
			d->data[index++] = data[i++];
		}
	}
	else {
		while (i++ < l) {
			if (index == MAX_DATA_LENGTH)
				index = 0;
			d->data[index++] = 0;
		}
	}
}

void InsertFuture(RainObject* o, double* v, long long time, long long count) {
	long long begin, end;
	begin = o->end + 1;
	end = o->end + time - o->time;
	CopyData(o, begin, 0, time - o->time - 1);
	CopyData(o, end, v, count);

	o->time = time + count - 1;
	o->end = end + count - 1;

	if (o->end >= MAX_DATA_LENGTH)
		o->full = 1;
	CheckIndex(o->end);
}

void InsertData(RainObject* o, double* v, long long time, long long count) {
	long long btime = o->full ? o->time - MAX_DATA_LENGTH : o->time - o->end,
		di = o->full ? (o->end + 1) % MAX_DATA_LENGTH : 0;//data begin index;

	if (!o->time)
		o->time = time - 1;

	if (time > o->time + MAX_DATA_LENGTH) {
		CopyData(o, 0, v, count);
		o->time = time + count;
		o->end = count - 1;
		o->full = 0;
	}
	else if (time > o->time) {
		InsertFuture(o, v, time, count);
	}
	else if (time + count > o->time) {
		CopyData(o, di + (time - btime), v, o->time - time);
		InsertFuture(o, v + (o->time - time), o->time + 1, count - (o->time - time));
	}
	else if (time >= btime) {
		CopyData(o, di + (time - btime), v, count);
	}
	else if (time + count > btime) {
		CopyData(o, di, v + (btime - time), count - (btime - time));
	}
}

void SearchData(RedisModuleCtx* ctx, RainObject* d, long long begin, long long end) {
	long long b = 0, e = 0, btime, dbegin, count, dend;

	btime = d->full ? d->time - MAX_DATA_LENGTH : d->time - d->end;//begin time
	dbegin = d->full ? (d->end + 1) % MAX_DATA_LENGTH : 0;//data begin index

	if (begin > d->time || end < btime || begin > end)
	{
		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		while (begin++ <= end)
			RedisModule_ReplyWithDouble(ctx, -1);
		count = end - begin + 1;
		RedisModule_ReplySetArrayLength(ctx, count < 0 ? 0 : count);
		return;
	}
	if (begin < btime)
		b = dbegin;
	else
		b = (dbegin + begin - btime);//% MAX_DATA_LENGTH;
	if (end > d->time)
		e = d->end;
	else
		e = (dbegin + end - btime);//% MAX_DATA_LENGTH;

	CheckIndex(b);
	CheckIndex(e);

	count = end - begin + 1;
	RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);

	while (begin++ < btime)
		RedisModule_ReplyWithDouble(ctx, -1);

	if (b > e) {
		while (b < MAX_DATA_LENGTH)
			RedisModule_ReplyWithDouble(ctx, d->data[b++]);
		b = 0;
	}
	while (b <= e)
		RedisModule_ReplyWithDouble(ctx, d->data[b++]);

	dend = d->time;
	while (dend++ < end)
		RedisModule_ReplyWithDouble(ctx, -1);

	RedisModule_ReplySetArrayLength(ctx, count);
}

double SearchSumData(RedisModuleCtx* ctx, RainObject* d, long long begin, long long end) {
	long long b = 0, e = 0, btime, dbegin, count, dend;
	double sum = 0.0;

	btime = d->full ? d->time - MAX_DATA_LENGTH : d->time - d->end;//begin time
	dbegin = d->full ? (d->end + 1) % MAX_DATA_LENGTH : 0;//data begin index

	if (begin > d->time || end < btime || begin > end)
		return;
	if (begin < btime)
		b = dbegin;
	else
		b = (dbegin + begin - btime);//% MAX_DATA_LENGTH;
	if (end > d->time)
		e = d->end;
	else
		e = (dbegin + end - btime);//% MAX_DATA_LENGTH;

	CheckIndex(b);
	CheckIndex(e);

	if (b > e) {
		while (b < MAX_DATA_LENGTH)
			sum += d->data[b++];
		b = 0;
	}
	while (b <= e)
		sum += d->data[b++];
	return sum;
}

long long RainDataLength(RainObject* o) {
	if (o->full) {
		return MAX_DATA_LENGTH;
	}
	else {
		return o->end + 1;
	}
}

int RainTypeSearch_RedisCommand(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 4) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != RainType) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long begin, end;
	if (RedisModule_StringToLongLong(argv[2], &begin) != REDISMODULE_OK ||
		RedisModule_StringToLongLong(argv[3], &end) != REDISMODULE_OK ||
		begin < 0 || end < 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a number");
	}

	RainObject* hto = RedisModule_ModuleTypeGetValue(key);
	if (hto)
		SearchData(ctx, hto, begin, end);
	else {
		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		RedisModule_ReplySetArrayLength(ctx, 0);
	}
	return REDISMODULE_OK;
}

int RainTypeSum_RedisCommand(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 4) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != RainType) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long begin, end;
	double result;
	if (RedisModule_StringToLongLong(argv[2], &begin) != REDISMODULE_OK ||
		RedisModule_StringToLongLong(argv[3], &end) != REDISMODULE_OK ||
		begin < 0 || end < 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a number");
	}

	RainObject* hto = RedisModule_ModuleTypeGetValue(key);
	if (hto) {
		result = SearchSumData(ctx, hto, begin, end);
		RedisModule_ReplyWithDouble(ctx, result);
	}
	else {
		RedisModule_ReplyWithDouble(ctx, 0);
	}
	return REDISMODULE_OK;
}

int RainTypeInsert_RedisCommand(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc <= 3 && argc > MAX_DAY_COUNT + 3) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != RainType) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	long long time;
	if ((RedisModule_StringToLongLong(argv[2], &time) != REDISMODULE_OK)) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a double");
	}

	if (time <= 0) {
		return RedisModule_ReplyWithError(ctx, "ERR invalid value: time can not be zero or negative");
	}

	double value[MAX_DAY_COUNT];
	for (size_t i = 3; i < argc; i++) {
		if ((RedisModule_StringToDouble(argv[i], &value[i - 3]) != REDISMODULE_OK)) {
			return RedisModule_ReplyWithError(ctx, "ERR invalid value: must be a double");
		}
	}

	/* Create an empty value object if the key is currently empty. */
	RainObject* hto;
	if (type == REDISMODULE_KEYTYPE_EMPTY) {
		hto = createRainArrayObject();
		RedisModule_ModuleTypeSetValue(key, RainType, hto);
	}
	else {
		hto = RedisModule_ModuleTypeGetValue(key);
	}
	InsertData(hto, value, time, (long long)argc - 3);

	RedisModule_ReplyWithLongLong(ctx, RainDataLength(hto));
	RedisModule_ReplicateVerbatim(ctx);
	return REDISMODULE_OK;
}

void RainTypeRdbSave(RedisModuleIO* rdb, void* value) {
	RainObject* hto = value;
	long long i = 0;
	RedisModule_SaveSigned(rdb, hto->time);
	RedisModule_SaveSigned(rdb, hto->end);
	RedisModule_SaveSigned(rdb, hto->full);

	//保存从0-end的数据
	while (i <= hto->end) {
		RedisModule_SaveDouble(rdb, hto->data[i++]);
	}

	if (hto->full) {
		while (i < MAX_DATA_LENGTH) {
			RedisModule_SaveDouble(rdb, hto->data[i++]);
		}
	}
}

void* RainTypeRdbLoad(RedisModuleIO* rdb, int encver) {
	if (encver != 0) {//版本号
		/* RedisModule_Log("warning","Can't load data with version %d", encver);*/
		return NULL;
	}
	uint64_t time = RedisModule_LoadUnsigned(rdb),
		end = RedisModule_LoadUnsigned(rdb),
		full = RedisModule_LoadUnsigned(rdb),
		i = 0;

	RainObject* hto = createRainArrayObject();
	hto->end = end;
	hto->time = time;
	hto->full = (char)full;

	while (i <= end) {
		double ele = RedisModule_LoadDouble(rdb);
		hto->data[i++] = (float)ele;
	}

	if (full) {
		while (i < MAX_DATA_LENGTH) {
			double ele = RedisModule_LoadDouble(rdb);
			hto->data[i++] = (float)ele;
		}
	}
	return hto;
}

char* F2S(double d, char* str)
{
	char str1[40];
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

void RainTypeAofRewrite(RedisModuleIO* aof, RedisModuleString* key, void* value) {
	RainObject* o = value;
	long long i = o->end + 1, time = o->time - o->end;
	char data[16];

	if (o->full) {
		time = o->time - MAX_DATA_LENGTH + 1;
		while (i < MAX_DATA_LENGTH) {
			F2S(o->data[i++], data);
			RedisModule_EmitAOF(aof, "raintype.insert", "slc", key, time++, data);
		}
		i = 0;
	}

	while (i < o->end) {
		F2S(o->data[i++], data);
		RedisModule_EmitAOF(aof, "raintype.insert", "slc", key, time++, data);
	}
}

int RainTypeNow_RedisCommand(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 2) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != RainType) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	RainObject* hto = RedisModule_ModuleTypeGetValue(key);
	if (hto != NULL)
		RedisModule_ReplyWithLongLong(ctx, hto->time);
	else
		RedisModule_ReplyWithNull(ctx);

	return REDISMODULE_OK;
}

void time2str(long long n, char* buf, size_t len)
{
	time_t time = n;
	struct tm p;
	time_t min = 1600000000, max = 2000000000;

	n *= 5 * 60;
	if (n > max)
	{
		buf[0] = '\0';
		return;
	}
	else if (n < min)
	{
		n *= 12;
		if (n > max || n < min)
		{
			buf[0] = '\0';
			return;
		}
	}
	gmtime_r(&p, &time);
	strftime(buf, len, "%Y-%m-%d %H:%M:%S", &p);
}

int RainTypeTime_RedisCommand(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	RedisModule_AutoMemory(ctx); /* Use automatic memory management. */

	if (argc != 2) return RedisModule_WrongArity(ctx);
	RedisModuleKey* key = RedisModule_OpenKey(ctx, argv[1], REDISMODULE_READ | REDISMODULE_WRITE);
	int type = RedisModule_KeyType(key);
	if (type != REDISMODULE_KEYTYPE_EMPTY && RedisModule_ModuleTypeGetType(key) != RainType) {
		return RedisModule_ReplyWithError(ctx, REDISMODULE_ERRORMSG_WRONGTYPE);
	}

	RainObject* hto = RedisModule_ModuleTypeGetValue(key);
	if (hto != NULL)
	{
		long long begin = hto->full ? hto->time - MAX_DATA_LENGTH : hto->time - hto->end;

		char* buf = RedisModule_Alloc(26);
		time2str(begin, buf, 26);
		RedisModule_ReplyWithArray(ctx, REDISMODULE_POSTPONED_ARRAY_LEN);
		RedisModule_ReplyWithCString(ctx, buf);
		time2str(hto->end, buf, 26);
		RedisModule_ReplyWithCString(ctx, buf);
		RedisModule_Free(buf);
		RedisModule_ReplySetArrayLength(ctx, 2);
	}
	else
		RedisModule_ReplyWithNull(ctx);

	return REDISMODULE_OK;
}

int RedisModule_OnLoad(RedisModuleCtx* ctx, RedisModuleString** argv, int argc) {
	REDISMODULE_NOT_USED(argv);
	REDISMODULE_NOT_USED(argc);

	if (RedisModule_Init(ctx, "raintype", 1, REDISMODULE_APIVER_1)
		== REDISMODULE_ERR) return REDISMODULE_ERR;

	RedisModuleTypeMethods tm = {
		.version = REDISMODULE_TYPE_METHOD_VERSION,
		.rdb_load = RainTypeRdbLoad,
		.rdb_save = RainTypeRdbSave,
		.aof_rewrite = RainTypeAofRewrite,
		.mem_usage = RainTypeMemUsage,
		.free = TypeFree,
		.digest = NULL
	};

	RainType = RedisModule_CreateDataType(ctx, "rain-type", 0, &tm);
	if (RainType == NULL) return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.insert",
		RainTypeInsert_RedisCommand, "write deny-oom", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.search",
		RainTypeSearch_RedisCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.sum",
		RainTypeSum_RedisCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.now",
		RainTypeNow_RedisCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	if (RedisModule_CreateCommand(ctx, "raintype.time",
		RainTypeTime_RedisCommand, "readonly", 1, 1, 1) == REDISMODULE_ERR)
		return REDISMODULE_ERR;

	return REDISMODULE_OK;
}


