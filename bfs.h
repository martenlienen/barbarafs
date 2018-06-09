#include <unistd.h>

#include <glib.h>

struct bfs_state {
  uid_t uid;
  gid_t gid;
  struct mailimap *imap;
  struct mailsmtp *smtp;
  GHashTable *files;
};

int bfs_main(int argc, char **argv);
