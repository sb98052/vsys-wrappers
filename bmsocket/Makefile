all: bmsocket.so

LIBDIR=../lib/

bmsocket.so: bmsocket.c $(LIBDIR)/fdpass.c
	gcc -shared -c bmsocket.c $(LIBDIR)/fdpass.c -o bmsocket.so
