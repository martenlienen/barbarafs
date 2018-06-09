#define _XOPEN_SOURCE

#include <stdio.h>

#include <errno.h>
#include <sys/stat.h>

#define FUSE_USE_VERSION 30
#include <fuse.h>

#include "bfs.h"
#include "bfs_file.h"
#include "bfs_imap.h"
#include "bfs_smtp.h"

static struct bfs_state state;

void bfs_hash_table_free_value(void *key, void *value, void *user_data) {
  bfs_file_free(value);
}

void bfs_state_free(struct bfs_state *state) {
  if (state->imap) {
    bfs_imap_free(state->imap);
  }

  if (state->smtp) {
    bfs_smtp_free(state->smtp);
  }

  g_hash_table_foreach(state->files, bfs_hash_table_free_value, NULL);
  g_hash_table_destroy(state->files);

  free(state);
}

void *bfs_init(struct fuse_conn_info *conn) {
  return fuse_get_context()->private_data;
}

void bfs_destroy(void *state) { bfs_state_free((struct bfs_state *)state); }

int bfs_getattr(const char *path, struct stat *st) {
  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  memset(st, 0, sizeof(struct stat));
  if (strcmp(path, "/") == 0) {
    st->st_mode = S_IFDIR | 0755;
    st->st_nlink = 1;
    st->st_uid = state->uid;
    st->st_gid = state->gid;
  } else {
    struct bfs_file *file = g_hash_table_lookup(state->files, path);
    if (file) {
      st->st_mode = S_IFREG | 0644;
      st->st_size = file->len;
      st->st_nlink = 1;
      st->st_uid = state->uid;
      st->st_gid = state->gid;
    } else {
      return -ENOENT;
    }
  }

  return 0;
}

int bfs_create(const char *path, mode_t mode, struct fuse_file_info *fi) {
  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  struct bfs_file *file = bfs_file_init(0, NULL);
  if (file) {
    // The hash table takes ownership of the key so we need to copy it
    size_t path_len = strlen(path) + 1;
    void *key = malloc(path_len * sizeof(char));
    if (!key) {
      bfs_file_free(file);
      return -ENOMEM;
    }

    memcpy(key, path, path_len);

    g_hash_table_insert(state->files, key, file);

    return 0;
  } else {
    return -EIO;
  }
}

int bfs_open(const char *path, struct fuse_file_info *fi) {
  printf("open %s\n", path);

  return 0;
}

int bfs_read(const char *path, char *out, size_t len, off_t off,
             struct fuse_file_info *fi) {
  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  struct bfs_file *file = g_hash_table_lookup(state->files, path);
  if (!file) {
    return -ENOENT;
  }

  if (off >= file->len) {
    return -EIO;
  }

  size_t read_len = file->len - off;
  if (len < read_len) {
    read_len = len;
  }
  memcpy(out, file->data + off, read_len * sizeof(char));

  return read_len;
}

int bfs_write(const char *path, const char *in, size_t len, off_t off,
              struct fuse_file_info *fi) {
  // Ensure that at least one byte is copied and file.data will be allocated
  if (len == 0) {
    return 0;
  }

  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  struct bfs_file *file = g_hash_table_lookup(state->files, path);
  if (!file) {
    return -ENOENT;
  }

  const size_t new_len = off + len;
  if (file->len < new_len) {
    char *new_data = realloc(file->data, new_len * sizeof(char));

    if (!new_data) {
      return -ENOMEM;
    }

    file->len = new_len;
    file->data = new_data;
  }

  memcpy(file->data + off, in, len * sizeof(char));

  return len;
}

int bfs_release(const char *path, struct fuse_file_info *fi) {
  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  struct bfs_file *file = g_hash_table_lookup(state->files, path);
  if (!file) {
    return -ENOENT;
  }

  printf("%s: %.*s\n", path, file->len, file->data);

  // bfs_smtp_send(state->smtp, data);

  return 0;
}

int bfs_unlink(const char *path) {
  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  gboolean removed = g_hash_table_remove(state->files, path);

  if (removed) {
    return 0;
  } else {
    return -1;
  }
}

int bfs_readdir(const char *path, void *buf, fuse_fill_dir_t filler, off_t off,
                struct fuse_file_info *fi) {
  if (strcmp(path, "/") != 0) {
    return -ENOENT;
  }

  struct fuse_context *ctx = fuse_get_context();
  struct bfs_state *state = (struct bfs_state *)ctx->private_data;

  char *filename;
  GHashTableIter iter;
  g_hash_table_iter_init(&iter, state->files);
  while (g_hash_table_iter_next(&iter, (void **)&filename, NULL)) {
    // Chop off the leading slash from the filename
    if (filler(buf, filename + 1, NULL, 0)) {
      break;
    }
  }

  return 0;
}

static const struct fuse_operations bfs_fuse_ops = {.init = bfs_init,
                                                    .destroy = bfs_destroy,
                                                    .getattr = bfs_getattr,
                                                    .create = bfs_create,
                                                    .open = bfs_open,
                                                    .read = bfs_read,
                                                    .write = bfs_write,
                                                    .release = bfs_release,
                                                    .unlink = bfs_unlink,
                                                    .readdir = bfs_readdir};

int bfs_main(int argc, char **argv) {
  state.uid = getuid();
  state.gid = getgid();
  state.imap = NULL;
  state.smtp = NULL;
  state.files = g_hash_table_new(g_str_hash, g_str_equal);

  if (!state.files) {
    printf("Out of memory.\n");
    return EXIT_FAILURE;
  }

  struct bfs_smtp_init_status smtp_status = bfs_smtp_init(&state.smtp);
  if (smtp_status.status) {
    char *smtp_message = NULL;
    bfs_smtp_init_strerror(smtp_status, &smtp_message);
    printf("%s", smtp_message);
    if (smtp_message) {
      free(smtp_message);
    }
    g_hash_table_destroy(state.files);
    return EXIT_FAILURE;
  } else {
    printf("Established SMTP connection.\n");
  }

  enum bfs_imap_init_error err = bfs_imap_init(&state.imap);
  if (err) {
    printf("Could not connect to your IMAP mailbox: %s\n",
           bfs_imap_init_strerror(err));
    bfs_smtp_free(state.smtp);
    g_hash_table_destroy(state.files);
    return EXIT_FAILURE;
  } else {
    printf("Established IMAP connection.\n");
  }

  return fuse_main(argc, argv, &bfs_fuse_ops, NULL);
}
