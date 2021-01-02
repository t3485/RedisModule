#include "db.h"

void save_v1(RedisDb* p, float v);
float load(RedisDb* p);
void flush(RedisDb* p);
inline int float_equal(float x, float y);

const float EPSINON = 0.00001;

RedisDb* db_init(RedisDb* p, RedisModuleIO* rdb, int ver) {
	p->last = p->count = 0;
	p->rdb = rdb;
	p->load = load;
	p->save = save_v1;
	p->flush = flush;
	p->ver = ver;

	return p;
}

/*
* 简单地压缩数据
* 将多个连续的相同的数据合并成 value splitvalue count
*/
void save_v1(RedisDb* p, float v) {
	if (p->count == 0) {
		p->last = v;
		p->count++;
	}
	else if (float_equal(p->last, v)) {
		p->count++;
	}
	else if (p->count > 4) {
		RedisModule_SaveFloat(p->rdb, p->last);
		RedisModule_SaveFloat(p->rdb, g_split);
		RedisModule_SaveUnsigned(p->rdb, p->count - 1);
		p->count = 0;
		save_v1(p, v);
	}
	else {
		while (p->count-- > 0) {
			RedisModule_SaveFloat(p->rdb, p->last);
		}
		p->count = 0;
		save_v1(p, v);
	}
}

float load(RedisDb* p) {
	float v = p->last;
	if (p->count > 0) {
		p->count--;
	}
	else {
		v = RedisModule_LoadFloat(p->rdb);
		if (g_split == v) {
			p->count = RedisModule_LoadUnsigned(p->rdb);
			return load(p);
		}
		p->last = v;
	}
	return v;
}

void flush(RedisDb* p) {
	if (p->count > 4) {
		RedisModule_SaveFloat(p->rdb, p->last);
		RedisModule_SaveFloat(p->rdb, g_split);
		RedisModule_SaveUnsigned(p->rdb, p->count - 1);
	}
	else {
		while (p->count-- > 0) {
			RedisModule_SaveFloat(p->rdb, p->last);
		}
	}
	db_init(p, p->rdb, p->ver);
}

int float_equal(float x, float y) {
	float e = x - y;
	return (e >= -EPSINON) && (e <= EPSINON);
}