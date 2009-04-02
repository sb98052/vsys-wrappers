#include <sys/socket.h>
#include <sys/un.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "fdpass.h"

#define VSYS_BMSOCKET "/vsys/fd_bmsocket.control"

int CreateLargeBufSocket(int recvbuf, int sndbuf)
{
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
    strncpy(addr.sun_path, VSYS_BMSOCKET,
            sizeof(addr.sun_path) - 1);

    if (connect(sfd, (struct sockaddr *) &addr,
                sizeof(struct sockaddr_un)) == -1) {
        perror("Could not connect to Vsys control socket");
        exit(-1);
    }

    /* passing the parameters */
    if (send(sfd, &recvbuf, sizeof(recvbuf), 0) != sizeof(recvbuf)) {
        perror("Could not connect to Vsys control socket");
        exit(-1);

    }
    if (send(sfd, &sndbuf, sizeof(sndbuf), 0) != sizeof(sndbuf)) {
        perror("Could not connect to Vsys control socket");
        exit(-1);
    }

    remotefd = receive_fd(sfd);
    return remotefd;
}

