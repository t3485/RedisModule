#include <stdio.h>
#include <malloc.h>
#include <time.h>
#include "hydrology.h"


void p(float v, void * a) {
	printf("%f ", v);
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
	free(o);
}
