#include <assert.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "stolen_from_fuse.h"

char *socket_name = "/vsys/fd_fusemount.control";

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

int mount(const char *source, const char *target, const char *filesystemtype,
        unsigned long mountflags, const void *data) {

  int fd = connect_socket();

  char buf[1024];
  sprintf( buf, "%08x\n", 0 );
  write( fd, buf, strlen(buf) );

  sprintf( buf, "%s\n%s\n%s\n%ld\n%s\n", source, target, filesystemtype,
	   mountflags, data );
  write( fd, buf, strlen(buf) );

  char inbuf[10];
  int n = read( fd, inbuf, 9 );
  inbuf[n] = '\0';

  int r;
  assert( sscanf( inbuf, "%08x\n", &r ) == 1);

  int fuse_fd = 0;
  if( r < 0 ) {
    errno = r;
    return -1;
  } else if( r > 0 ) {
    // get the fd
    fuse_fd = receive_fd(fd);

    // what was the old fd?
    int old_fd;
    char extra[1024];
    int s = sscanf( data, "fd=%d,%s", &old_fd, extra );
    assert( dup2( fuse_fd, old_fd ) == old_fd );

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

