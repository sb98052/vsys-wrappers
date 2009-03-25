#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "stolen_from_fuse.h"

char *socket_name = "/vsys/fd_fusemount.control";
unsigned int arg_length = 128;

void send_argument(int control_channel_fd, char *source) {
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
  assert( connect( fd, (struct sockaddr *) &addr, len ) == 0 );
  return fd;
}

void do_umount( char *const argv[], int n, int fd ) {

  // write the length
  char buf[1024];
  sprintf( buf, "%08x\n", n );
  write( fd, buf, strlen(buf) );

  // now write each arg
  int i;
  for( i = 0; i < n; i++ ) {
    assert( strlen(argv[i]) < 1024 );
    sprintf( buf, "%s\n", argv[i] );
    write( fd, buf, strlen(buf) );
  }

  char inbuf[10];
  int n2 = read( fd, inbuf, 10 );
  inbuf[n2] = '\0';

  int r = atoi(inbuf);

}

int umount2( const char *mnt, int flags ) {
 
  int fd = connect_socket();

  const char *argv[3];
  argv[0] = "fusermount";
  argv[1] = "-u";
  argv[2] = mnt;

  do_umount( (char **const) argv, 3, fd );

  close(fd);
 
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

  char buf[1024];

  send_argument(fd, source);
  send_argument(fd, target );
  send_argument(fd, filesystemtype );
  send_argument(fd, data );

  old_fuse_fd = get_magic_fd (data);

  send_fd(fd, old_fuse_fd);

  if (fuse_fd == -1) {
      printf ("Reroutemount: Could not identify FUSE fd: %d\n", fuse_fd);
      exit(1);
  }

  new_fuse_fd=receive_fd(fd);

  if (new_fuse_fd == -1) {
      printf ("Reroutemount: Fusemount returned bad fd: %d\n", fuse_fd);
      exit(1);
  }

  if( dup2(new_fuse_fd, old_fuse_fd ) != new_fuse_fd ) {
      printf ("Could not duplicate returned file descriptor\n");
      exit(1);
  }

  close(fd);
  return 0;

}

int execv( const char *path, char *const argv[] ) {

  if( strstr( path, "fusermount" ) == NULL ) {
    return execv( path, argv );
  }

  // also make sure this is an unmount . . .
  int n = 0;
  char *arg = argv[n];
  int found_u = 0;
  while( arg != NULL ) {
    if( strcmp( arg, "-u" ) == 0 ) {
      found_u = 1;
      break;
    }
    arg = argv[++n];
  }

  if( !found_u ) {
    return execv( path, argv );
  }

  // Have root do any fusermounts we need done
  int fd = connect_socket();

  do_umount( argv, n, fd );

  exit(0);

}

