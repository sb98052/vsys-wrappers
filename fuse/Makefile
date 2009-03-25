all: fuse_vsys_wrapper.so

fuse_vsys_wrapper.so: fdpass.c reroutemount.c
	gcc -shared fdpass.c reroutemount.c -o fuse_vsys_wrapper.so
