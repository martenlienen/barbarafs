#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <libetpan.h>

#include "bfs_smtp.h"

void bfs_smtp_init_strerror(struct bfs_smtp_init_status status, char **msg) {
  char *buffer;
  size_t msg_size;

  switch (status.status) {
  case BFS_SMTP_INIT_SUCCESS:
    buffer = "Success.";
    *msg = malloc(sizeof(char) * strlen(buffer));
    if (*msg) {
      strcpy(*msg, buffer);
    }
    break;
  case BFS_SMTP_INIT_ALLOCATE_ERROR:
    buffer = "Could not allocate an SMTP connection.";
    *msg = malloc(sizeof(char) * strlen(buffer));
    if (*msg) {
      strcpy(*msg, buffer);
    }
    break;
  case BFS_SMTP_INIT_CONNECT_ERROR:
    buffer = "Could not connect to the SMTP server: %s.";
    msg_size = sizeof(char) * (strlen(buffer) + strlen(status.msg) - 2);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  case BFS_SMTP_INIT_EHLO_ERROR:
    buffer = "Failed to initiate a connection: %s.";
    msg_size = sizeof(char) * (strlen(buffer) + strlen(status.msg) - 2);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  case BFS_SMTP_INIT_AUTH_ERROR:
    buffer = "Could not authenticate: %s.";
    msg_size = sizeof(char) * (strlen(buffer) + strlen(status.msg) - 2);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  default:
    buffer = "Unknown error.";
    *msg = malloc(sizeof(char) * strlen(buffer));
    if (*msg) {
      strcpy(*msg, buffer);
    }
  }
}

struct bfs_smtp_init_status bfs_smtp_init(bfs_smtp **smtp) {
  int err;
  struct bfs_smtp_init_status status;

  struct mailsmtp *smtp_h = mailsmtp_new(0, NULL);
  if (!smtp_h) {
    status = (struct bfs_smtp_init_status){BFS_SMTP_INIT_ALLOCATE_ERROR, NULL};
    goto alloc_error;
  }

  err = mailsmtp_ssl_connect(smtp_h, "sslout.df.eu", 465);
  if (err) {
    status = (struct bfs_smtp_init_status){BFS_SMTP_INIT_CONNECT_ERROR,
                                           mailsmtp_strerror(err)};
    goto connect_error;
  }

  err = mailesmtp_ehlo(smtp_h);
  if (err) {
    status = (struct bfs_smtp_init_status){BFS_SMTP_INIT_EHLO_ERROR,
                                           mailsmtp_strerror(err)};
    goto ehlo_error;
  }

  err = mailsmtp_auth(smtp_h, "mail@exmaple.com", "password");
  if (err) {
    status = (struct bfs_smtp_init_status){BFS_SMTP_INIT_AUTH_ERROR,
                                           mailsmtp_strerror(err)};
    goto auth_error;
  }

  *smtp = smtp_h;
  return (struct bfs_smtp_init_status){BFS_SMTP_INIT_SUCCESS, NULL};

auth_error:
ehlo_error:
connect_error:
  mailsmtp_free(smtp_h);
alloc_error:
  return status;
}

void bfs_smtp_free(bfs_smtp *smtp) { mailsmtp_free(smtp); }

void bfs_smtp_send_strerror(struct bfs_smtp_send_status status, char **msg) {
  char *buffer;
  size_t msg_size;

  switch (status.status) {
  case BFS_SMTP_SEND_SUCCESS:
    buffer = "Success.";
    *msg = malloc(sizeof(char) * strlen(buffer));
    if (*msg) {
      strcpy(*msg, buffer);
    }
    break;
  case BFS_SMTP_SEND_INIT_ERROR:
    buffer = "Could not initiate mail: %s.";
    msg_size = sizeof(char) * strlen(buffer) - 2 + strlen(status.msg);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  case BFS_SMTP_SEND_RECIPIENT_ERROR:
    buffer = "Could not add recipient: %s.";
    msg_size = sizeof(char) * strlen(buffer) - 2 + strlen(status.msg);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  case BFS_SMTP_SEND_DATA_ERROR:
    buffer = "Could not initiate data section: %s.";
    msg_size = sizeof(char) * strlen(buffer) - 2 + strlen(status.msg);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  case BFS_SMTP_SEND_TRANSMISSION_ERROR:
    buffer = "Could not send data: %s.";
    msg_size = sizeof(char) * strlen(buffer) - 2 + strlen(status.msg);
    *msg = malloc(msg_size);
    if (*msg) {
      snprintf(*msg, msg_size, buffer, status.msg);
    }
    break;
  }
}

struct bfs_smtp_send_status bfs_smtp_send(struct mailsmtp *session,
                                          const char *data) {
  int err;

  err = mailesmtp_mail(session, "mail@example.com", 0, NULL);
  if (err) {
    return (struct bfs_smtp_send_status){BFS_SMTP_SEND_INIT_ERROR,
                                         mailsmtp_strerror(err)};
  }

  err = mailesmtp_rcpt(session, "marten.lienen@gmail.com",
                       MAILSMTP_DSN_NOTIFY_FAILURE | MAILSMTP_DSN_NOTIFY_DELAY,
                       NULL);
  if (err) {
    return (struct bfs_smtp_send_status){BFS_SMTP_SEND_RECIPIENT_ERROR,
                                         mailsmtp_strerror(err)};
  }

  err = mailsmtp_data(session);
  if (err) {
    return (struct bfs_smtp_send_status){BFS_SMTP_SEND_DATA_ERROR,
                                         mailsmtp_strerror(err)};
  }

  err = mailsmtp_data_message(session, data, 1000);
  if (err) {
    return (struct bfs_smtp_send_status){BFS_SMTP_SEND_TRANSMISSION_ERROR,
                                         mailsmtp_strerror(err)};
  }

  return (struct bfs_smtp_send_status){BFS_SMTP_SEND_SUCCESS, NULL};
}
