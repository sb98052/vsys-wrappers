all: fuse_vsys_wrapper.so

fuse_vsys_wrapper.so: ../lib/fdpass.c reroutemount.c
	gcc -g -shared -I../lib ../lib/fdpass.c reroutemount.c -o fuse_vsys_wrapper.so
