#include <stdint.h>

struct bfs_file {
  int32_t len;
  char *data;
};

struct bfs_file *bfs_file_init(int32_t len, char *data);
void bfs_file_free(struct bfs_file *file);
