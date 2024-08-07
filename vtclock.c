/*
 * vtclock.c -- Program to display a large digital clock.
 *
 * Original code by Rob Hoeft.
 * Enhancements by Darren Embry.
 * Fonts by Darren Embry.
 *
 */

#include <string.h>
#include <ncurses.h>
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>
#include <stdio.h>
#include <getopt.h>
#include <stdlib.h>

#include "font0.h"
#include "font1.h"
#include "font2.h"
#include "font3.h"
#include "digitalfont0.h"
#include "msg.h"
#include "vtclock.h"
#include "figlet.h"

void pollkey(void);

#define AT_LEAST(a, b) do { if (a < b) a = b; } while(0)

#define MAKE_DIGIT_WINDOW(sw, type)                                     \
  do {                                                                  \
    if (config->type) {                                                 \
      sw = subwin(cl, config->type->digit_height, config->type->digit_width, \
                  starty + (VTCLOCK_ALIGN * (cl_height - config->type->digit_height) / 2), startx); \
      startx += config->type->digit_width;                              \
    } else {                                                            \
      sw = NULL;                                                        \
    }                                                                   \
  } while(0)
#define MAKE_COLON_WINDOW(sw, type)                                     \
  do {                                                                  \
    if (config->type) {                                                 \
      sw = subwin(cl, config->type->digit_height, config->type->colon_width, \
                  starty + (VTCLOCK_ALIGN * (cl_height - config->type->digit_height) / 2), startx); \
      startx += config->type->colon_width;                              \
    } else {                                                            \
      sw = NULL;                                                        \
    }                                                                   \
  } while(0)
#define DRAW_DIGIT(sw, type, digit)                                 \
  do {                                                              \
    if (sw) {                                                       \
      vtclock_print_string(sw, 0, 0, config->type->digits[digit]);  \
    }                                                               \
  } while(0)
#define DRAW_COLON(sw, type)                                \
  do {                                                      \
    if (sw) {                                               \
      vtclock_print_string(sw, 0, 0, config->type->colon);  \
    }                                                       \
  } while(0)
#define DRAW_BLANK_COLON(sw, type)                                      \
  do {                                                                  \
    if (sw) {                                                           \
      vtclock_print_blank_version_of_string(sw, 0, 0, config->type->colon); \
    }                                                                   \
  } while(0)

vtclock_config vtclock_config_1 = {
  &vtclock_font_0, &vtclock_font_0, &vtclock_font_0,
  &vtclock_font_0, &vtclock_font_0
};

vtclock_config vtclock_digital_config_1 = {
  &vtclock_digital_font_0, &vtclock_digital_font_0, &vtclock_digital_font_0,
  &vtclock_digital_font_0, &vtclock_digital_font_0
};

vtclock_config vtclock_config_2 = {
  &vtclock_font_1, &vtclock_font_1, &vtclock_font_2,
  &vtclock_font_1, NULL
};

vtclock_config vtclock_config_3 = {
  &vtclock_font_3, &vtclock_font_3, &vtclock_font_3,
  &vtclock_font_3, &vtclock_font_3
};

vtclock_config vtclock_config_4 = {
  &vtclock_font_2, &vtclock_font_2, &vtclock_font_2,
  &vtclock_font_2, &vtclock_font_2
};

/* 0 = top; 1 = middle; 2 = bottom */
#define VTCLOCK_ALIGN 0

static int vtclock_inverse = 0;
static char vtclock_char = 0; /* always use this character, if set */

void
version()
{
  fprintf(stdout,
          "vtclock %d.%d.%d\n",
          VTCLOCK_VERSION_MAJOR,
          VTCLOCK_VERSION_MINOR,
          VTCLOCK_VERSION_PATCHLEVEL);
}

void
small_sleep()
{
  static struct timeval timeout;
  timeout.tv_sec = 0;
  timeout.tv_usec = 50000;  /* 0.05 secs */
  select(0, NULL, NULL, NULL, &timeout);
}

struct timeval prev;
struct timeval curr;

void
mydelay()
/* sleep until second changes.  works via polling.  close enough
   for government work. */
{
  gettimeofday(&prev, NULL);
  while (1) {
    gettimeofday(&curr, NULL);
    if (curr.tv_sec > prev.tv_sec)
      return;
    pollkey();
    prev = curr;
  }
}

void
mydelay_half()
/* sleep until the half-second mark.  also works via polling.
   also close enough for government work. */
{
  while (1) {
    gettimeofday(&curr, NULL);
    if (curr.tv_usec >= 500000)
      return;
    pollkey();
  }
}

void
vtclock_print_string(WINDOW *win, int y, int x,
                     char *str)
{
  wclear(win); /* Fixes -v and -c mode screen over-writing */
  if (vtclock_inverse) 
  {
    char *p;
    mvwin(win, y, x);
    for (p = str; *p; ++p) {
      if (iscntrl(*p)) {
        waddch(win, *p);
      } else {
        waddch(win, ' ' | (isspace(*p) ? A_NORMAL : A_REVERSE));
      }
    }
  }
  else if (vtclock_char) 
  {
    char *p;
    mvwin(win, y, x);
    for (p = str; *p; ++p) {
      waddch(win, iscntrl(*p) ? *p : isspace(*p) ? *p : vtclock_char);
    }
  }
  else 
  {
    mvwprintw(win, y, x, "%s", str);
  }
}

void
vtclock_print_blank_version_of_string(WINDOW *win, int y, int x, char *str)
{
  char *p;
  wclear(win); /* Fixes screen over-writing in -v and -c modes */
  mvwin(win, y, x);
  for (p = str; *p; ++p) {
    if (iscntrl(*p)) {
      waddch(win, *p);
    } else {
      waddch(win, ' ');
    }
  }
}

void
usage() {
  fprintf(stdout,
          "usage: vtclock [option ...]\n"
          "  -h         help\n"
          "  -b/-B      turn bouncing on/off                             (default on)\n"
          "  -d <secs>  delay between each bounce step                   (default 30 secs)\n"
          "  -1/-2/-3/-4/-5  select a font                               (default font #1)\n"
          "  -c <char>  use specified character for font drawing\n"
          "  -v         use bright solid inverse-video blocks\n"
          "  -C/-V      use normal font drawing characters               (default)\n"
          "  -k/-K      blinking colons on/off                           (default off)\n"
          "  -f <file>  shows one line at a time from filename\n"
          "  -p <cmd>   shows one line at a time from output of command  (via /bin/sh -c)\n"
          "  -D <secs>  delay between each message line                  (default 5 secs)\n"
          "  -F <font>  use a figlet font\n"
          "  -F -       use default figlet font\n"
    );
  version();
}

int
main(int argc, char **argv) {
  WINDOW *cl;                   /* used to draw the clock */
  WINDOW *h1, *h2, *m1, *m2, *s1, *s2, *c1, *c2;
  /* subcomponents of cl */
  WINDOW *cld;                  /* used to erase the clock */

  vtclock_config *config = &vtclock_config_2;
  vtclock_config *temp_config;

  int cl_height, cl_width;
  int y, x;                     /* clock window position */
  int startx, starty;           /* for placing sub-windows */
  int updown, leftright;        /* dy, dx */
  int futurex, futurey;         /* temp. for bounds checking */
  int waitfor = 0;              /* bouncing-related counter */
  
  time_t t_time;
  struct tm *tm_time;           /* extract HH:MM:SS from here */

  int vtclock_bounce = 1;
  int vtclock_bounce_delay = 30;
  int vtclock_msg_delay = 5;
  int blinking_colons = 0;
  int msg_from_pipe = 0;

  int show_message_line = 0;
  char *msg_filename = NULL;

  char *msg = NULL;
  WINDOW *msgw = NULL;

  struct figlet_options the_figlet_options = {
  font_name: NULL
  };

  {
    int ch;
    extern char *optarg;
    extern int optind;
    extern int optopt;
    extern int opterr;
    opterr = 1;
    optind = 1;
    while ((ch = getopt(argc, argv, "hbBd:D:12345vVkKc:Cf:p:F:")) != -1) {
      switch (ch) {
      case 'h':
        usage();
        exit(0);
      case 'c':
        vtclock_inverse = 0;
        vtclock_char = optarg[0];
        break;
      case 'C':
        vtclock_inverse = 0;
        vtclock_char = 0;
        break;
      case 'v':
        vtclock_inverse = 1;
        vtclock_char = 0;
        break;
      case 'V':
        vtclock_inverse = 0;
        vtclock_char = 0;
        break;
      case 'b':
        vtclock_bounce = 1;
        break;
      case 'B':
        vtclock_bounce = 0;
        break;
      case 'd':
        vtclock_bounce_delay = atoi(optarg);
        break;
      case 'D':
        vtclock_msg_delay = atoi(optarg);
        break;
      case '1':
        config = &vtclock_config_2;
        break;
      case '2':
        config = &vtclock_config_1;
        break;
      case '3':
        config = &vtclock_digital_config_1;
        break;
      case '4':
        config = &vtclock_config_3;
        break;
      case '5':
        config = &vtclock_config_4;
        break;
      case 'F':
        if (optarg) {
          if (!strcmp(optarg, "-")) {
            if (the_figlet_options.font_name) {
              free(the_figlet_options.font_name);
              the_figlet_options.font_name = NULL;
            }
          } else {
            the_figlet_options.font_name = strdup(optarg);
          }
        }
        temp_config = generate_figlet_config(&the_figlet_options);
        if (temp_config)
          config = temp_config;
        break;
      case 'k':
        blinking_colons = 1;
        break;
      case 'K':
        blinking_colons = 0;
        break;
      case 'f':
        show_message_line = 1;
        msg_from_pipe = 0;
        if (msg_filename != NULL)
          free(msg_filename);
        msg_filename = strdup(optarg);
        break;
      case 'p':
        show_message_line = 1;
        msg_from_pipe = 1;
        if (msg_filename != NULL)
          free(msg_filename);
        msg_filename = strdup(optarg);
        break;
      case '?':
      default:
        usage();
        exit(2);
      }
    }
  }
  
  argc -= optind;
  argv += optind;

  initscr();
  cbreak();
  noecho();
  nonl();
  wtimeout(curscr, 50);

  cl_height = 0;
  if (config->hour)   AT_LEAST(cl_height, config->hour->digit_height);
  if (config->minute) AT_LEAST(cl_height, config->minute->digit_height);
  if (config->second) AT_LEAST(cl_height, config->second->digit_height);
  if (config->colon1) AT_LEAST(cl_height, config->colon1->digit_height);
  if (config->colon2) AT_LEAST(cl_height, config->colon2->digit_height);

  cl_width
    = (config->hour   ? config->hour->digit_width * 2 : 0)
    + (config->minute ? config->minute->digit_width * 2 : 0)
    + (config->second ? config->second->digit_width * 2 : 0)
    + (config->colon1 ? config->colon1->colon_width : 0)
    + (config->colon2 ? config->colon2->colon_width : 0);

  /* BUG WORKAROUND: cl_height + 2 instead of clheight */
  if ((LINES < (cl_height + 2)) || (COLS < cl_width)) {
    endwin();
    fprintf(stderr, "(LINES=%d COLS=%d) screen too small!\n",
            LINES, COLS);
    exit(3);
  }

  if (show_message_line) {
    if (LINES >= (cl_height + 4)) {
      init_message(cl_width, msg_filename, msg_from_pipe, vtclock_msg_delay);
      cl_height += 2;
    }
  }
  
  y = (LINES - cl_height) / 2;
  x = (COLS - cl_width) / 2;

  startx = x;
  starty = y;

  updown = (LINES > cl_height) ? 1 : 0;
  leftright = (COLS > cl_width) ? 1 : 0;

  cl  = newwin(cl_height, cl_width, y, x);
  cld = newwin(cl_height, cl_width, y, x);

  MAKE_DIGIT_WINDOW(h1, hour);
  MAKE_DIGIT_WINDOW(h2, hour);
  MAKE_COLON_WINDOW(c1, colon1);
  MAKE_DIGIT_WINDOW(m1, minute);
  MAKE_DIGIT_WINDOW(m2, minute);
  MAKE_COLON_WINDOW(c2, colon2);
  MAKE_DIGIT_WINDOW(s1, second);
  MAKE_DIGIT_WINDOW(s2, second);

  if (show_message_line) {
    msgw = subwin(cl, 1, cl_width, y + cl_height - 1, x);
  }

  curs_set(0);

  gettimeofday(&curr, NULL);

  while (1) {
    tm_time = localtime(&(curr.tv_sec));

    DRAW_DIGIT(h1, hour, tm_time->tm_hour / 10);
    DRAW_DIGIT(h2, hour, tm_time->tm_hour % 10);
    DRAW_DIGIT(m1, minute, tm_time->tm_min / 10);
    DRAW_DIGIT(m2, minute, tm_time->tm_min % 10);
    DRAW_DIGIT(s1, second, tm_time->tm_sec / 10);
    DRAW_DIGIT(s2, second, tm_time->tm_sec % 10);
    DRAW_COLON(c1, colon1);
    DRAW_COLON(c2, colon2);

    if (show_message_line) {
      char *msg = get_next_message();
      if (msg != NULL) {
        /* clear the line */
        mvwprintw(msgw, 0, 0, "%*s", cl_width, "");
        /* display the new message */
        mvwprintw(msgw, 0, (cl_width - strlen(msg)) / 2, "%s", msg);
      }
    }
    
    if (vtclock_bounce) {
      ++waitfor;
      if (waitfor >= vtclock_bounce_delay) {
        if (tm_time->tm_sec == 0) {
          /* don't move the clock on :00 - this can be slow */
          waitfor = vtclock_bounce_delay - 1;
        } else {
          /* erase old */
          mvwin(cld, y, x);
          wnoutrefresh(cld);

          /* bouncy bouncy */
          futurex = x + leftright;
          futurey = y + updown;
          if ((futurex == 0) && (futurey == 0)) {
            futurex = x + (leftright *= -1);
            futurey = y + (updown *= -1);
          } else {
            if ((futurex < 0) || (futurex > (COLS - cl_width))) {
              futurex = x + (leftright *= -1);
            }
            if ((futurey < 0) || (futurey > (LINES - cl_height))) {
              futurey = y + (updown *= -1);
            }
          }
          x = futurex;
          y = futurey;

          waitfor = 0;
        }
      }
    }

    mvwin(cl, y, x);
    wnoutrefresh(cl);
    doupdate();

    if (blinking_colons) {
      mydelay_half();
      mvwin(cl, y, x); /* Position the blank colon */
      DRAW_BLANK_COLON(c1, colon1);
      DRAW_BLANK_COLON(c2, colon2);
      wnoutrefresh(cl);
      doupdate();
    }

    mydelay();
  }

  endwin();
  return 0;
}

void
pollkey(void)
{
  int key;
  key = wgetch(curscr);
  switch (key) {
  case 12:      /* Control-L */
  case 18:      /* Control-R */
  case KEY_REFRESH:
    redrawwin(curscr);
    wrefresh(curscr);
  }
}
