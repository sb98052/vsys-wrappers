#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fdpass.h"
#include <dlfcn.h>

#define VSYS_PACKETSEER "/vsys/fd_packetseer.control"

int (*socket_orig)(int f, int p, int s);

int _init_pslib() {
    socket_orig = &socket;
    printf("Stored value of socket");
}

int socket(int f, int p, int s)
{
    if (!socket_orig) {
        void *handle = dlopen("/lib/libc.so.6",RTLD_LAZY);
        if (!handle) {
            fprintf(stderr,"Error loading libc.so.6\n");
            return -1;
        }
        socket_orig = dlsym(handle, "socket");
        if (!socket_orig) {
            fprintf(stderr,"Error loading socket symbol");
            return -1;
        }
        fprintf(stderr,"Socket call: %x",socket_orig);
    }

    if (f == PF_PACKET) {
        int sfd;
        struct sockaddr_un addr;
        int remotefd;

        sfd = socket(AF_UNIX, SOCK_STREAM, 0);
        if (sfd == -1) {
            perror("Could not create UNIX socket\n");
            exit(-1);
        }

        memset(&addr, 0, sizeof(struct sockaddr_un));
        /* Clear structure */
        addr.sun_family = AF_UNIX;
        strncpy(addr.sun_path, VSYS_PACKETSEER,
                sizeof(addr.sun_path) - 1);

        if (connect(sfd, (struct sockaddr *) &addr,
                    sizeof(struct sockaddr_un)) == -1) {
            perror("Could not connect to Vsys control socket");
            exit(-1);
        }

        remotefd = receive_fd(sfd);
        return remotefd;
    }
    else
        return (*socket_orig)(f, p, s);
}

