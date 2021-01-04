#ifndef _HYDROLOGY_H
#define _HYDROLOGY_H 

#include "common.h"

struct RainArrayObject {
	float data[g_data_length];
	long long end;//���һ�����ݵ�����λ��
	long long time;//���һ�����ݵ�ʱ��
	int count, length, total;
	char full;
};
typedef struct RainArrayObject RainObject;

struct SearchResult {
	long long index;//����λ��
	long long pre;//ǰ׺��С
	long long suf;//��׺��С
	long long size;//��С
};

void hyd_init(RainObject* o);
void hyd_init_length(RainObject* o, int count, int length);
void hyd_insert(RainObject * o, double* v, long long time, long long count);
void hyd_search(RainObject * o, long long begin, long long end, struct SearchResult* r);
float hyd_sum(RainObject * o, long long begin, long long end);
long long hyd_len(RainObject * o);
void hyd_each(RainObject* o, struct SearchResult* r, void (*fn)(float, void*), void* p);
int checkIndex(RainObject* o, int i);

#endif