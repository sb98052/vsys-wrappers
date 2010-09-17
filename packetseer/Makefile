all: packetseer.so

LIBDIR=../lib/
INCLUDEDIR=../lib/

packetseer.so: packetseer.c $(LIBDIR)/fdpass.c
	gcc -I$(INCLUDEDIR) -shared -ldl packetseer.c $(LIBDIR)/fdpass.c -o packetseer.so
