dllmain.so: dllmain.o hydrology.o module.o db.o common.o
	gcc -shared -o dllmain.so dllmain.o hydrology.o module.o db.o common.o
dllmain.o: dllmain.c
	gcc -fPIC -c dllmain.c -o dllmain.o
hydrology.o: hydrology.c
	gcc -fPIC -c hydrology.c
module.o: module.c
	gcc -fPIC -c module.c
db.o: db.c
	gcc -fPIC -O3 -c db.c
common.o: common.c
	gcc -fPIC -c common.c
clean:
	rm -rf *.o