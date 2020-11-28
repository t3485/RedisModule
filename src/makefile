dllmain.so: dllmain.o hydrology.o module.o
	gcc -shared -o dllmain.so dllmain.o hydrology.o module.o
dllmain.o: dllmain.c
	gcc -fPIC -c dllmain.c -o dllmain.o
hydrology.o: hydrology.c
	gcc -fPIC -c hydrology.c
module.o: module.c
	gcc -fPIC -c module.c
clean:
	rm -rf *.o