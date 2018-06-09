typedef struct mailimap bfs_imap;

enum bfs_imap_init_error {
  BFS_IMAP_INIT_ALLOCATE_ERROR = 1,
  BFS_IMAP_INIT_CONNECT_ERROR,
  BFS_IMAP_INIT_LOGIN_ERROR,
  BFS_IMAP_INIT_INBOX_ERROR
};

char *bfs_imap_init_strerror(enum bfs_imap_init_error err);

int bfs_imap_init(bfs_imap **imap);
void bfs_imap_free(bfs_imap *imap);
