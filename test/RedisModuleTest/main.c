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
	i = CheckIndex(i);

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

	struct SearchResult r;

	for (size_t i = 0; i < 12; i++)
		hyd_insert(o, v + i, 100 + i, 1);
	hyd_insert(o, v, 103, 1);
	hyd_insert(o, v, 121, 1);
	hyd_insert(o, v, 119, 12);

	free(o);
}
