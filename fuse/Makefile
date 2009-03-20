all: reroutemount.c reroutemount_server.c stolen_from_fuse.c
	gcc -g -c stolen_from_fuse.c -o stolen_from_fuse.o
	gcc -g stolen_from_fuse.o reroutemount_server.c -o reroutemount_server
	gcc -g -shared stolen_from_fuse.o reroutemount.c -o reroutemount.so
