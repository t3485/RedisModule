#ifndef  HYDROLOGY_H
#define HYDROLOGY_H 

#define MAX_DAY_COUNT 288
#define MAX_DATA_LENGTH (MAX_DAY_COUNT * 92)
#define CheckIndex(i) if(i >= MAX_DATA_LENGTH){ i %= MAX_DATA_LENGTH; }

struct RainArrayObject {
	float data[MAX_DATA_LENGTH];
	long long end;//���һ�����ݵ�����λ��
	long long time;//���һ�����ݵ�ʱ��
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
void hyd_insert(RainObject * o, double* v, long long time, long long count);
void hyd_search(RainObject * o, long long begin, long long end, struct SearchResult* r);
float hyd_sum(RainObject * o, long long begin, long long end);
long long hyd_len(RainObject * o);
void hyd_each(RainObject* o, struct SearchResult* r, void (*fn)(float, void*), void* p);

#endif