#include "hydrology.h"

inline int checkIndex(RainObject* o, int i) {
	return i >= o->total ? i % o->total : i;
}

void hyd_init(RainObject* o) {
	o->time = 0;
	o->end = -1;
	o->full = 0;

	o->count = g_max_count;
	o->length = g_data_length;
	o->total = o->count * o->length;
}

void hyd_init_length(RainObject* o, int count, int length) {
	o->time = 0;
	o->end = -1;
	o->full = 0;

	o->count = count;
	o->length = length;
	o->total = o->count * o->length;
}

void hyd_copy(RainObject* o, int index, double* data, long long l) {
	int i = 0;
	if (data) {
		while (i < l) {
			if (index >= o->total)
				index = checkIndex(o, index);
			o->data[index++] = data[i++];
		}
	}
	else {
		while (i++ < l) {
			if (index >= o->total)
				index = checkIndex(o, index);
			o->data[index++] = 0;
		}
	}
}

void hyd_insert_future(RainObject* o, double* v, long long time, long long count) {
	long long begin, end;
	begin = o->end + 1;
	end = o->end + time - o->time;
	hyd_copy(o, begin, 0, time - o->time - 1);
	hyd_copy(o, end, v, count);

	o->time = time + count - 1;
	o->end = end + count - 1;

	if (o->end >= o->total)
		o->full = 1;
	o->end = checkIndex(o, o->end);
}

void hyd_insert(RainObject* o, double* v, long long time, long long count) {
	if (!o->time)
		o->time = time - 1;

	long long btime = o->full ? o->time - o->total + 1 : o->time - o->end,
		di = o->full ? (o->end + 1) % o->total : 0;//data begin index;

	/*            o                o->total      v
	*             |                      |              |
	*  +-----------------+               +           +------+
	*  +-----------------+               +           +------+ */
	if (time > o->time + o->total) {
		hyd_copy(o, 0, v, count);
		o->time = time + count - 1;
		o->end = count - 1;
		o->full = 0;
	}
	/*            o              v
	*             |              |
	*  +-----------------+    +------+
	*  +-----------------+    +------+ */
	else if (time > o->time) {
		hyd_insert_future(o, v, time, count);
	}
	/*            o         v
	*             |         |
	*  +-----------------------+
	*  +-----------------------+ */
	else if (time + count > o->time) {
		long long repert = o->time - time + 1;
		hyd_copy(o, di + (time - btime), v, repert);
		hyd_insert_future(o, v + repert, o->time + 1, count - repert);
	}
	else if (time >= btime) {
		hyd_copy(o, di + (time - btime), v, count);
	}
	else if (time + count > btime) {
		hyd_copy(o, di, v + (btime - time), count - (btime - time));
	}
}

void hyd_search(RainObject* o, long long begin, long long end, struct SearchResult* r) {
	long long bgtime, di;

	bgtime = o->full ? o->time - o->total : o->time - o->end;//begin time
	di = o->full ? checkIndex(o, o->end + 1) : 0;//data begin index

	if (begin > o->time || end < bgtime || begin > end) {
		r->pre = end - begin + 1;
		r->suf = r->size = r->index = 0;
		return;
	}

	if (begin < bgtime) {
		r->pre = bgtime - begin;
		r->index = di;
	}
	else {
		r->pre = 0;
		r->index = (di + begin - bgtime);
		r->index = checkIndex(o, r->index);
	}

	if (end > o->time)
		r->suf = end - o->time;
	else r->suf = 0;
	r->size = end - begin + 1 - r->pre - r->suf;
}

float hyd_sum(RainObject* o, long long begin, long long end) {
	struct SearchResult r;
	float sum = 0;
	long long b, e;

	hyd_search(o, begin, end, &r);

	b = r.index;
	e = r.index + r.size;
	while (b < e) {
		if (b >= o->total)
			sum += o->data[b - o->total];
		else sum += o->data[b];
		b++;
	}
	return sum;
}

long long hyd_len(RainObject* o) {
	if (o->full)
		return o->total;
	else return o->end + 1;
}

void hyd_each(RainObject* o, struct SearchResult* r, void (*fn)(float, void*), void* p) {
	long long b = 0, e;

	while (b++ < r->pre)
		fn(0, p);
	b = r->index;
	e = r->index + r->size;
	while (b < e) {
		if (b >= o->total)
			fn(o->data[b - o->total], p);
		else fn(o->data[b], p);
		b++;
	}
	b = 0;
	while (b++ < r->suf)
		fn(0, p);
}