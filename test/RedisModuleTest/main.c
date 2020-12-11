#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "../../src/hydrology.h"


void p(float v, void* a) {
	printf("%f ", v);
}

int redis_save(void* value) {
	int index = 0;
	RainObject* hto = value;

	long long i = hto->full ? hto->end + 1 : 0,
		e = hto->full ? MAX_DATA_LENGTH - 1 : hto->end;
	CheckIndex(i);

	while (i <= e) {
		index++;
		i++;
	}

	if (hto->full) {
		i = 0;
		while (i <= hto->end) {
			index++;
			i++;
		}
	}
	return index;
}

int main() {
	RainObject* o = malloc(sizeof(RainObject));
	hyd_init(o);
	double v[288];

	for (size_t i = 0; i < 288; i++)
		v[i] = i;

	hyd_insert(o, v, 0, 288);
	hyd_insert(o, v, 578, 288);
	hyd_insert(o, v, 100, 288);

	struct SearchResult r;

	hyd_search(o, 1, 11, &r);
	printf("%lld %lld %lld %lld %lld\n", r.index, r.pre, r.size, r.suf, hyd_len(o));
	hyd_each(o, &r, p, NULL);
	printf("\n--------------\n");
	hyd_search(o, 60, 110, &r);
	printf("%lld %lld %lld %lld %lld\n", r.index, r.pre, r.size, r.suf, hyd_len(o));
	hyd_each(o, &r, p, NULL);
	printf("\n--------------\n");
	hyd_search(o, 200, 220, &r);
	printf("%lld %lld %lld %lld %lld\n", r.index, r.pre, r.size, r.suf, hyd_len(o));
	hyd_each(o, &r, p, NULL);
	printf("\n--------------\n");
	hyd_search(o, 350, 400, &r);
	printf("%lld %lld %lld %lld %lld\n", r.index, r.pre, r.size, r.suf, hyd_len(o));
	hyd_each(o, &r, p, NULL);

	for (size_t i = 0; i < 288; i++)
		v[i] = i + 700;
	hyd_insert(o, v, 700, 288);

	o->end = MAX_DATA_LENGTH - 1;
	o->full = 1;
	printf("count : %d\n", redis_save(o));

	free(o);


}
