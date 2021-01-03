
#include "hydrology.h"

void throw(){
	*(int*)0 = 0;
}

void assertFloatEqual(float x, float y) {
	if (x == y)
		return;
	throw();
}

void assertCharEqual(char x, char y) {
	if (x == y)
		return;
	throw();
}

void assertIntEqual(int x, int y) {
	if (x == y)
		return;
	throw();
}

void assertlongEqual(long long x, long long y) {
	if (x == y)
		return;
	throw();
}

void CheckInsert() {
	double data[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	RainObject* o = (RainObject*)malloc(sizeof(RainObject));
	hyd_init_length(o, 1, 12);
	assertIntEqual(o->total, 12);

	hyd_insert(o, data, 100, 1);
	assertCharEqual(o->full, 0);
	assertIntEqual(o->end, 0);
	assertFloatEqual(o->data[0], 0);

	hyd_insert(o, data, 100, 12);
	assertCharEqual(o->full, 0);
	assertIntEqual(o->end, 11);
	for (size_t i = 0; i < 12; i++)
		assertFloatEqual(o->data[i], i);

	free(o);
}

void CheckInsertBefore() {
	double data[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	RainObject* o = (RainObject*)malloc(sizeof(RainObject));
	hyd_init_length(o, 1, 12);
	hyd_insert(o, data, 100, 12);

	hyd_insert(o, data, 94, 12);
	assertCharEqual(o->full, 0);
	assertIntEqual(o->end, 11);
	for (size_t i = 0; i < 6; i++)
		assertFloatEqual(o->data[i], i + 6);
	for (size_t i = 6; i < 12; i++)
		assertFloatEqual(o->data[i], i);

	free(o);
}

void CheckInsertFarBefore() {
	double data[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	RainObject* o = (RainObject*)malloc(sizeof(RainObject));
	hyd_init_length(o, 1, 12);
	hyd_insert(o, data, 100, 12);

	hyd_insert(o, data, 80, 12);
	for (size_t i = 0; i < 12; i++)
		assertFloatEqual(o->data[i], i);

	free(o);
}

void CheckInsertFarFuture() {
	double data[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	RainObject* o = (RainObject*)malloc(sizeof(RainObject));
	hyd_init_length(o, 1, 12);
	hyd_insert(o, data, 100, 12);

	hyd_insert(o, data + 1, 200, 1);
	assertCharEqual(o->full, 0);
	assertIntEqual(o->end, 0);
	assertFloatEqual(o->data[0], 1);

	free(o);
}

void CheckInsertFuture() {
	double data[12] = { 0,1,2,3,4,5,6,7,8,9,10,11 };
	RainObject* o = (RainObject*)malloc(sizeof(RainObject));
	hyd_init_length(o, 1, 12);
	hyd_insert(o, data, 100, 12);

	hyd_insert(o, data, 106, 12);
	assertCharEqual(o->full, 1);
	for (size_t i = 0; i < 6; i++)
		assertFloatEqual(o->data[i], i + 6);
	for (size_t i = 6; i < 12; i++)
		assertFloatEqual(o->data[i], i - 6);

	free(o);
}

int main() {
	CheckInsertFarBefore();
	CheckInsertFarFuture();
	CheckInsertFuture();
	CheckInsertBefore();
	CheckInsert();

	printf("%s\n", "pass");
}