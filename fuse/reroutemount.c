#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "fdpass.h"

char *socket_name = "/vsys/fd_fusemount.control";

unsigned int arg_length = 128;

void send_argument(int control_channel_fd, const char *source) {
    int sent;
    sent=send(control_channel_fd, source, arg_length, 0);
    if (sent<arg_length) {
        printf("Error receiving arguments over the control buffer\n");
        exit(1);
    }
}

int connect_socket() {
  int fd = socket( AF_UNIX, SOCK_STREAM, 0 );
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strcpy( addr.sun_path, socket_name );
  int len = strlen(socket_name) + sizeof(addr.sun_family);
  printf("Connecting to %s\n", socket_name);
  assert( connect( fd, (struct sockaddr *) &addr, len ) == 0 );
  return fd;
}

int get_magic_fd (char *data) {
    char *ptr;
    int fd;

    data[arg_length-1]='\0';
    ptr = strstr(data,"fd=");
    if (!ptr)
        return -1;

    // Found two fd= expressions
    if (strstr(ptr+3,"fd="))
        return -1;

    if (*(ptr+3)!='\0') {
        sscanf(ptr+3,"%d",&fd);
        return fd;
    }
    else
        return -1;
}

int mount(const char *source, const char *target, const char *filesystemtype,
        unsigned long mountflags, const void *data) {
  int fd = connect_socket();
  int old_fuse_fd, new_fuse_fd;
  int dupfd;

  char buf[1024];

  send_argument(fd, source);
  send_argument(fd, target );
  send_argument(fd, filesystemtype );
  send_argument(fd, data );

  old_fuse_fd = get_magic_fd (data);


  if (old_fuse_fd == -1) {
      printf ("Reroutemount: Could not identify FUSE fd: %d\n", old_fuse_fd);
      exit(1);
  }

  send_fd(fd, old_fuse_fd);
  new_fuse_fd=receive_fd(fd);

  if (new_fuse_fd == -1) {
      printf ("Reroutemount: Fusemount returned bad fd: %d\n", new_fuse_fd);
      exit(1);
  }

  if( (dupfd=dup2(new_fuse_fd, old_fuse_fd )) != old_fuse_fd ) {
      printf ("Could not duplicate returned file descriptor: %d\n",dupfd);
      exit(1);
  }

  close(fd);
  return 0;

}
