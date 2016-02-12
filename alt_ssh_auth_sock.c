#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <glob.h>

#include <sys/socket.h>
#include <sys/un.h>
#include <unistd.h>
#include <errno.h>

#include <stdio.h>

static char* (*_original_getenv)(const char *name) = NULL;

static void _alt_ssh_auth_sock_init() __attribute__((constructor));
static void _alt_ssh_auth_sock_init() {
  _original_getenv = dlsym(RTLD_NEXT, "getenv");
}

int check_socket(const char* socket_file_path) {
  struct sockaddr_un addr;
  addr.sun_family = AF_UNIX;
  strncpy(addr.sun_path, socket_file_path, sizeof(addr.sun_path) / sizeof(char));

  int fd = socket(AF_UNIX, SOCK_STREAM, 0);
  int ret = connect(fd, (struct sockaddr *)&addr, sizeof(struct sockaddr_un));
  close(fd);
  return ret;
}

char* getenv(const char *name) {
  if (!_original_getenv) {
    _alt_ssh_auth_sock_init();
  }

  if (strcmp(name, "SSH_AUTH_SOCK") != 0) {
    // Ignore others
    return _original_getenv(name);
  }

  char* ssh_auth_sock = _original_getenv("SSH_AUTH_SOCK");
  if (!ssh_auth_sock) {
    // do nothing if SSH_AUTH_SOCK is not set
    return ssh_auth_sock;
  }

  char* alt_ssh_auth_sock = _original_getenv("ALT_SSH_AUTH_SOCK");
  if (!alt_ssh_auth_sock) {
    // do nothing if ALT_SSH_AUTH_SOCK is not set
    return ssh_auth_sock;
  }

  // check if SSH_AUTH_SOCK is alive
  if (check_socket(ssh_auth_sock) == 0) {
    // alive!
    return ssh_auth_sock;
  }

  // list alternative sockets
  glob_t pglob;
  if (glob(alt_ssh_auth_sock, GLOB_NOSORT, NULL, &pglob) != 0) {
    globfree(&pglob);
  }

  int i;
  for (i = 0; i < pglob.gl_pathc; ++i) {
    if (check_socket(pglob.gl_pathv[i]) == 0) {
      break;
    }
  }
  if (i < pglob.gl_pathc) {
    // overwrite SSH_AUTH_SOCK
    setenv("SSH_AUTH_SOCK", pglob.gl_pathv[i], 1);
    ssh_auth_sock = _original_getenv("SSH_AUTH_SOCK");
  }
  globfree(&pglob);

  return ssh_auth_sock;
}

