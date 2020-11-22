dllmain.so: dllmain.o hydrology.o module.o
	gcc -o dllmain.so dllmain.o hydrology.o module.o
dllmain.o: dllmain_linux.c
	gcc -c dllmain_linux.c -o dllmain.o
hydrology.o: hydrology.c
	gcc -c hydrology.c
module.o: module.c
	gcc -c module.c
clean:
	rm -rf *.o