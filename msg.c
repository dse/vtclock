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
static int msg_args_is_pipe = 0;
static int msg_delay = 5;

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
  if ((now - last_ready) >= msg_delay) {
    last_ready = now;
    return 1;
  }
  return 0;
}

void
init_message (int cols, int argc, char **argv, int is_pipe, int delay)
{
  if (msg != NULL) return;
  msg = (char *)malloc(cols + 1);
  msg[0] = '\0';
  msg_argc = argc;
  msg_argv = argv;
  msg_cols = cols;
  msg_args_is_pipe = is_pipe;
  msg_delay = delay;
}

static FILE *msg_file;

FILE *
open_msg_file (void)
{
  if (msg_file != NULL) return msg_file;
  if (msg_args_is_pipe) {
    msg_file = popen(msg_argv[0], "r");
  } else {
    msg_file = fopen(msg_argv[0], "r");
  }
  if (msg_file == NULL) return NULL;
  return msg_file;
}

char *
get_next_message (void)
{
  if (msg == NULL) return NULL;	/* sanity */
  if (msg_argv[0] == NULL) return NULL;	/* sanity */
  if (!message_ready()) return NULL;

  if (open_msg_file() == NULL) return NULL;
  if (fgets(msg, msg_cols, msg_file) != NULL) return msg;
  fclose(msg_file);
  msg_file = NULL;
  if (open_msg_file() == NULL) return NULL;
  if (fgets(msg, msg_cols, msg_file) != NULL) return msg;
  return NULL;
}

