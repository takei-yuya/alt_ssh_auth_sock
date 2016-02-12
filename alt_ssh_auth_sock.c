#define _GNU_SOURCE
#include <stdlib.h>
#include <dlfcn.h>
#include <string.h>

#include <sys/stat.h>
#include <errno.h>

#include <glob.h>

static char* (*_original_getenv)(const char *name) = NULL;

static void _ssh_agent_switcher_init() __attribute__((constructor));
static void _ssh_agent_switcher_init() {
  _original_getenv = dlsym(RTLD_NEXT, "getenv");
}

char* getenv(const char *name) {
  if (!_original_getenv) {
    _ssh_agent_switcher_init();
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
  struct stat st;
  if (lstat(ssh_auth_sock, &st) == 0) {
    // alive!
    return ssh_auth_sock;
  }

  // list alternative sockets
  glob_t pglob;
  if (glob(alt_ssh_auth_sock, GLOB_NOSORT, NULL, &pglob) != 0) {
    globfree(&pglob);
  }
  if (pglob.gl_pathc == 0) {
    // no one
    globfree(&pglob);
    return ssh_auth_sock;
  }

  // overwrite SSH_AUTH_SOCK
  setenv("SSH_AUTH_SOCK", pglob.gl_pathv[0], 1);
  ssh_auth_sock = _original_getenv("SSH_AUTH_SOCK");
  globfree(&pglob);

  return ssh_auth_sock;
}

