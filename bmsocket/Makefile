all: bmsocket.so

LIBDIR=../lib/
INCLUDEDIR=../lib/

bmsocket.so: bmsocket.c $(LIBDIR)/fdpass.c
	gcc -I$(INCLUDEDIR) -shared bmsocket.c $(LIBDIR)/fdpass.c -o bmsocket.so
