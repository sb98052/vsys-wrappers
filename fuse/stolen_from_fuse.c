#include <assert.h>
#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mount.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "stolen_from_fuse.h"

// Most of this code is stolen from FUSE 2.7.4

/** Function to pass a file descriptor over a UNIX socket.
  * 
  * Sends the file descriptor fd over the channel sock_fd.
  *
  ***/
int rrm_send_fd(int sock_fd, int fd)
{
	int retval;
	struct msghdr msg;
	struct cmsghdr *p_cmsg;
	struct iovec vec;
	size_t cmsgbuf[CMSG_SPACE(sizeof(fd)) / sizeof(size_t)];
	int *p_fds;
	char sendchar = 0;

	msg.msg_control = cmsgbuf;
	msg.msg_controllen = sizeof(cmsgbuf);
	p_cmsg = CMSG_FIRSTHDR(&msg);
	p_cmsg->cmsg_level = SOL_SOCKET;
	p_cmsg->cmsg_type = SCM_RIGHTS;
	p_cmsg->cmsg_len = CMSG_LEN(sizeof(fd));
	p_fds = (int *) CMSG_DATA(p_cmsg);
	*p_fds = fd;
	msg.msg_controllen = p_cmsg->cmsg_len;
	msg.msg_name = NULL;
	msg.msg_namelen = 0;
	msg.msg_iov = &vec;
	msg.msg_iovlen = 1;
	msg.msg_flags = 0;
	/* "To pass file descriptors or credentials you need to send/read at
	 * least one byte" (man 7 unix) */
	vec.iov_base = &sendchar;
	vec.iov_len = sizeof(sendchar);
	while ((retval = sendmsg(sock_fd, &msg, 0)) == -1 && errno == EINTR);
	if (retval != 1) {
		perror("sending file descriptor");
		return -1;
	}
	return 0;
}


/* return value:
 * >= 0	 => fd
 * -1	 => error
 */
int rrm_receive_fd(int fd)
{
	struct msghdr msg;
	struct iovec iov;
	char buf[1];
	int rv;
	size_t ccmsg[CMSG_SPACE(sizeof(int)) / sizeof(size_t)];
	struct cmsghdr *cmsg;

	iov.iov_base = buf;
	iov.iov_len = 1;

	msg.msg_name = 0;
	msg.msg_namelen = 0;
	msg.msg_iov = &iov;
	msg.msg_iovlen = 1;
	/* old BSD implementations should use msg_accrights instead of
	 * msg_control; the interface is different. */
	msg.msg_control = ccmsg;
	msg.msg_controllen = sizeof(ccmsg);

	while(((rv = recvmsg(fd, &msg, 0)) == -1) && errno == EINTR);
	if (rv == -1) {
		perror("recvmsg");
		return -1;
	}
	if(!rv) {
		/* EOF */
		return -1;
	}

	cmsg = CMSG_FIRSTHDR(&msg);
	if (!cmsg->cmsg_type == SCM_RIGHTS) {
		fprintf(stderr, "got control message of unknown type %d\n",
			cmsg->cmsg_type);
		return -1;
	}
	return *(int*)CMSG_DATA(cmsg);
}

int rrm_fuse_mnt_umount(const char *progname, const char *mnt, int lazy)
{
	int res;
	int status;
	
	res = umount2(mnt, lazy ? 2 : 0);
	if (res == -1)
	  fprintf(stderr, "%s: failed to unmount %s: %s\n",
		  progname, mnt, strerror(errno));
	return res;

}

static int rrm_try_open(const char *dev, char **devp, int silent)
{
	int fd = open(dev, O_RDWR);
	if (fd != -1) {
		*devp = strdup(dev);
		if (*devp == NULL) {
		  fprintf(stderr, "failed to allocate memory\n" );
				
			close(fd);
			fd = -1;
		}
	} else if (errno == ENODEV ||
		   errno == ENOENT)/* check for ENOENT too, for the udev case */
		return -2;
	else if (!silent) {
		fprintf(stderr, "failed to open %s: %s\n", dev,
			strerror(errno));
	}
	return fd;
}

static int rrm_try_open_fuse_device(char **devp)
{
	int fd;
	int err;

	//drop_privs();
	fd = rrm_try_open(FUSE_DEV_NEW, devp, 0);
	//restore_privs();
	if (fd >= 0)
		return fd;

	err = fd;
	fd = rrm_try_open(FUSE_DEV_OLD, devp, 1);
	if (fd >= 0)
		return fd;

	return err;
}

int rrm_open_fuse_device(char **devp)
{
	int fd = rrm_try_open_fuse_device(devp);
	if (fd >= -1)
		return fd;

	fprintf(stderr,
		"fuse device not found, try 'modprobe fuse' first\n");

	return -1;
}
