all: fuse_vsys_wrapper.so

fuse_vsys_wrapper.so: stolen_from_fuse.c reroutemount.c
	gcc -shared stolen_from_fuse.c reroutemount.c -o fuse_vsys_wrapper.so
