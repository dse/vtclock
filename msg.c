/*
 * msg.c
 *
 * Stuff for the one-line message line in the vtclock.
 */

#include "msg.h"
#include <stdlib.h>
#include <time.h>
#include <stdio.h>

static char *msg = NULL;
static int msg_argc = 0;
static int msg_cols = 0;
static char **msg_argv = NULL;

int
message_ready (void)
{
  static time_t last_ready = 0;
  static time_t now;
  now = time(NULL);
  if (last_ready == 0) {
    last_ready = now;
    return 1;
  }
  if ((now - last_ready) >= 5) {
    last_ready = now;
    return 1;
  }
  return 0;
}

void
init_message (int cols, int argc, char **argv)
{
  if (msg != NULL) return;
  msg = (char *)malloc(cols + 1);
  msg[0] = '\0';
  msg_argc = argc;
  msg_argv = argv;
  msg_cols = cols;
}

char *
get_next_message (void)
{
  static FILE *fh = NULL;

  if (msg == NULL) return NULL;	/* sanity */
  if (msg_argv[0] == NULL) return NULL;	/* sanity */
  if (!message_ready()) return NULL;

  if (fh == NULL) {
    fh = fopen(msg_argv[0], "r");
    if (fh == NULL) return NULL;
  }

  if (fh != NULL) {
    if (fgets(msg, msg_cols, fh) != NULL) {
      return msg;
    }
    fclose(fh);
    fh = fopen(msg_argv[0], "r");
    if (fh == NULL) return NULL;
    if (fgets(msg, msg_cols, fh) != NULL) {
      return msg;
    }
    return NULL;
  }
}

