#include <libetpan.h>

#include "bfs_imap.h"

char *bfs_imap_init_strerror(enum bfs_imap_init_error error) {
  switch (error) {
  case BFS_IMAP_INIT_ALLOCATE_ERROR:
    return "Could not allocate an IMAP connection.";
  case BFS_IMAP_INIT_CONNECT_ERROR:
    return "Could not connect to your mailbox.";
  case BFS_IMAP_INIT_LOGIN_ERROR:
    return "Mailbox credentials are wrong.";
  case BFS_IMAP_INIT_INBOX_ERROR:
    return "Could not open inbox.";
  default:
    return "Unknown error.";
  }
}

int bfs_imap_init(bfs_imap **imap) {
  int err = 0;
  struct mailimap *imap_h = mailimap_new(0, NULL);
  if (!imap_h) {
    err = BFS_IMAP_INIT_ALLOCATE_ERROR;
    goto alloc_error;
  }

  err = mailimap_ssl_connect(imap_h, "sslin.df.eu", 993);
  if (err < 0 || err > 2) {
    err = BFS_IMAP_INIT_CONNECT_ERROR;
    goto connect_error;
  }

  err = mailimap_login(imap_h, "logwatch@cqql.de", "Uzn=Jppugkc4");
  if (err) {
    err = BFS_IMAP_INIT_LOGIN_ERROR;
    goto login_error;
  }

  err = mailimap_select(imap_h, "INBOX");
  if (err) {
    err = BFS_IMAP_INIT_INBOX_ERROR;
    goto inbox_error;
  }

  *imap = imap_h;
  return 0;

inbox_error:
  mailimap_logout(imap_h);
login_error:
connect_error:
  mailimap_free(imap_h);
alloc_error:
  return err;
}

void bfs_imap_free(bfs_imap *imap) {
  mailimap_logout(imap);
  mailimap_free(imap);
}
