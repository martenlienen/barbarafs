#include <stdlib.h>

#include "bfs_file.h"

struct bfs_file *bfs_file_init(int32_t len, char *data) {
  struct bfs_file *file = malloc(sizeof(struct bfs_file));

  if (file) {
    file->len = len;
    file->data = data;
  }

  return file;
}

void bfs_file_free(struct bfs_file *file) { free(file); }
