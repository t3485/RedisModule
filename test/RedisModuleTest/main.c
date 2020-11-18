#include <stdio.h>
#include <malloc.h>
#include <time.h>

typedef long long PORT_ULONGLONG;

#define MAX_DAY_COUNT 288
#define MAX_DATA_LENGTH (MAX_DAY_COUNT * 3)
#define CheckIndex(i) if(i >= MAX_DATA_LENGTH){ i %= MAX_DATA_LENGTH; }

struct RainArrayObject {
	float data[MAX_DATA_LENGTH];
	PORT_ULONGLONG end, time, btime;
	char full, span;
};

typedef struct RainArrayObject RainObject;


void SearchData(struct RainArrayObject* d, long long begin, long long end) {
	long long b = 0, e = 0, btime, dbegin, count, dend;

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

	count = end - begin;
	while (begin++ < btime)
		printf("%lf", -1.0);

	if (b > e) {
		while (b < MAX_DATA_LENGTH)
			printf("%lf ", d->data[b++]);
		b = 0;
	}
	while (b <= e)
		printf("%lf ", d->data[b++]);

	dend = d->time;
	while (dend++ <= end)
		printf("%lf ", -1.0);
}

void CopyData(RainObject* d, long long index, double* data, long long l) {
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

void InsertFuture(RainObject* o, double* v, PORT_ULONGLONG time, PORT_ULONGLONG count) {
	PORT_ULONGLONG begin, end;
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

void InsertData(RainObject* o, double* v, PORT_ULONGLONG time, PORT_ULONGLONG count) {
	PORT_ULONGLONG btime = o->full ? o->time - MAX_DATA_LENGTH : o->time - o->end,
		di = o->full ? (o->end + 1) % MAX_DATA_LENGTH : 0;//data begin index;

	if (!o->time)
		o->time = time - 1;

	if (time > o->time + MAX_DATA_LENGTH) {
		CopyData(o, 0, v, count);
		o->time = time + count;
		o->end = count - 1;
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
	gmtime_s(&p, &n);
	strftime(buf, len, "%Y-%m-%d %H:%M:%S", &p);
}

int main() {
	RainObject* o = malloc(sizeof(RainObject));
	o->time = 0;
	o->end = -1;
	o->full = 0;
	double v[288];

	for (size_t i = 0; i < 288; i++)
	{
		v[i] = i;
	}

	InsertData(o, v, 0, 288);
	InsertData(o, v, 578, 288);


	//InsertData(o, v, 100, 288);

	//SearchData(o, 1, 11);
	//printf("\n--------------\n");
	//SearchData(o, 60, 110);
	//printf("\n--------------\n");
	//SearchData(o, 200, 220);
	//printf("\n--------------\n");
	//SearchData(o, 350, 400);

	//for (size_t i = 0; i < 288; i++)
	//{
	//	v[i] = i + 700;
	//}
	//InsertData(o, v, 700, 288);
	free(o);

	char buf[26];
	time2str(5352095, buf, 26);
	printf("%s", buf);
}
