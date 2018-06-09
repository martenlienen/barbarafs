typedef struct mailsmtp bfs_smtp;

enum bfs_smtp_init_error {
  BFS_SMTP_INIT_SUCCESS,
  BFS_SMTP_INIT_ALLOCATE_ERROR,
  BFS_SMTP_INIT_CONNECT_ERROR,
  BFS_SMTP_INIT_EHLO_ERROR,
  BFS_SMTP_INIT_AUTH_ERROR
};

struct bfs_smtp_init_status {
  enum bfs_smtp_init_error status;
  const char *msg;
};

void bfs_smtp_init_strerror(struct bfs_smtp_init_status status, char **msg);

enum bfs_smtp_send_error {
  BFS_SMTP_SEND_SUCCESS,
  BFS_SMTP_SEND_INIT_ERROR,
  BFS_SMTP_SEND_RECIPIENT_ERROR,
  BFS_SMTP_SEND_DATA_ERROR,
  BFS_SMTP_SEND_TRANSMISSION_ERROR
};

struct bfs_smtp_send_status {
  enum bfs_smtp_send_error status;
  const char *msg;
};

void bfs_smtp_send_strerror(struct bfs_smtp_send_status status, char **msg);

struct bfs_smtp_init_status bfs_smtp_init(bfs_smtp **smtp);
void bfs_smtp_free(bfs_smtp *smtp);

struct bfs_smtp_send_status bfs_smtp_send(struct mailsmtp *session,
                                          const char *data);
