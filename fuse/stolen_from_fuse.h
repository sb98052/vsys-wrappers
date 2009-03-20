#define FUSE_DEV_OLD "/proc/fs/fuse/dev"
#define FUSE_DEV_NEW "/dev/fuse"

int rrm_receive_fd(int fd);
int rrm_send_fd(int sock_fd, int fd);
int rrm_open_fuse_device(char **devp);
int rrm_fuse_mnt_umount(const char *progname, const char *mnt, int lazy);

