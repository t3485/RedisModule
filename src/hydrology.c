#include "hydrology.h"

void hyd_init(RainObject* o) {
	o->time = 0;
	o->end = -1;
	o->full = 0;
}

void hyd_copy(RainObject* o, int index, double* data, long long l) {
	int i = 0;
	if (data) {
		while (i < l) {
			if (index == MAX_DATA_LENGTH)
				index = 0;
			o->data[index++] = data[i++];
		}
	}
	else {
		while (i++ < l) {
			if (index == MAX_DATA_LENGTH)
				index = 0;
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

	if (o->end >= MAX_DATA_LENGTH)
		o->full = 1;
	CheckIndex(o->end);
}

void hyd_insert(RainObject* o, double* v, long long time, long long count) {
	long long btime = o->full ? o->time - MAX_DATA_LENGTH : o->time - o->end,
		di = o->full ? (o->end + 1) % MAX_DATA_LENGTH : 0;//data begin index;

	if (!o->time)
		o->time = time - 1;
	/*            o                MAX_DATA_LENGTH      v
	*             |                      |              |
	*  +-----------------+               +           +------+
	*  +-----------------+               +           +------+ */
	if (time > o->time + MAX_DATA_LENGTH) {
		hyd_copy(o, 0, v, count);
		o->time = time + count;
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
		hyd_copy(o, di + (time - btime), v, o->time - time);
		hyd_insert_future(o, v + (o->time - time), o->time + 1, count - (o->time - time));
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

	bgtime = o->full ? o->time - MAX_DATA_LENGTH : o->time - o->end;//begin time
	di = o->full ? (o->end + 1) % MAX_DATA_LENGTH : 0;//data begin index

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
		CheckIndex(r->index);
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

	b = r.index - r.pre;
	while (b < r.index) {
		if (b < 0)
			sum += o->data[b + MAX_DATA_LENGTH];
		else sum += o->data[b];
		b++;
	}
	e = r.index + r.size;
	while (b < e) {
		if (b >= MAX_DATA_LENGTH)
			sum += o->data[b - MAX_DATA_LENGTH];
		else sum += o->data[b];
		b++;
	}
	e = r.index + r.suf;
	while (b < e) {
		if (b >= MAX_DATA_LENGTH)
			sum += o->data[b - MAX_DATA_LENGTH];
		else sum += o->data[b];
		b++;
	}
	return sum;
}

long long hyd_len(RainObject* o) {
	if (o->full)
		return MAX_DATA_LENGTH;
	else return o->end + 1;
}

void hyd_each(RainObject* o, struct SearchResult* r, void (*fn)(float, void*), void* p) {
	long long b, e;

	b = r->index - r->pre;
	while (b < r->index) {
		if (b < 0)
			fn(o->data[b + MAX_DATA_LENGTH], p);
		else fn(o->data[b], p);
		b++;
	}
	e = r->index + r->size;
	while (b < e) {
		if (b >= MAX_DATA_LENGTH)
			fn(o->data[b - MAX_DATA_LENGTH], p);
		else fn(o->data[b], p);
		b++;
	}
	e = r->index + r->suf;
	while (b < e) {
		if (b >= MAX_DATA_LENGTH)
			fn(o->data[b - MAX_DATA_LENGTH], p);
		else fn(o->data[b], p);
		b++;
	}
}