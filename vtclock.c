/*
 * vtclock.c
 * 
 * Program to display a large digital clock.
 *
 * Font "stolen" from figlet
 *
 * Original code by Rob Hoeft.
 * Enhancements by Darren Embry.
 *
 */

#include <ncurses.h>
#include "digits.h"
#include <time.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <time.h>
#include <ctype.h>

#define VTCLOCK_BOUNCE 1

/* VTCLOCK_INVERSE code is buggy right now. */
#define VTCLOCK_INVERSE 0

void 
mydelay() 
     /* sleep until second changes.  does this by polling every
      * 1/10 second.  close enough for government work. */
{
  static struct timeval timeout;
  static time_t prevsecs = (time_t)0;
  time_t secs;
  while (1) {
    time(&secs);
    if (prevsecs != secs) {
      prevsecs = secs;
      return;
    }
    timeout.tv_sec = 0;
    timeout.tv_usec = 100000;	/* 0.1 secs */
    select(0,NULL,NULL,NULL,&timeout);
    prevsecs = secs;
  }
}

void
vtclock_print_string(WINDOW *win, int y, int x,
		     char *str)
{
  if (VTCLOCK_INVERSE)
    {
      char *p;
      mvwin(win,y,x);
      for (p = str; *p; ++p) {
	if (iscntrl(*p)) {
	  waddch(win,*p);
	} else {
	  waddch(win,' '|(isspace(*p)?A_NORMAL:A_REVERSE));
	}
      }
    }
  else
    {
      mvwprintw(win,y,x,str);
    }
}

int
main() {
  WINDOW *cl;			/* used to draw the clock */
  WINDOW *h1, *h2, *m1, *m2, *s1, *s2, *c1, *c2;
				/* subcomponents of cl */
  WINDOW *cld;			/* used to erase the clock */

  int di_width, cl_height, co_width, cl_width;
  int y, x;			/* position */
  int startx, starty;		/* for placing sub-windows */
  int updown, leftright;	/* dy, dx */
  int futurex, futurey;		/* temp. for bounds checking */
  int waitfor = 0;		/* bouncing-related counter */

  time_t t_time;
  struct tm *tm_time;		/* extract HH:MM:SS from here */

  initscr();

  cl_height = DIGIT_HEIGHT;
  di_width = DIGIT_WIDTH;
  co_width = COLON_WIDTH;
  cl_width = co_width * 2 + di_width * 6;
  
  if ((LINES < cl_height) || (COLS < cl_width)) {
    endwin();
    fprintf(stderr,"(LINES=%d COLS=%d) screen too small!\n",
	    LINES,COLS);
    exit(3);
  }

  y = (LINES - cl_height) / 2;
  x = (COLS - cl_width) / 2;

  startx = x;
  starty = y;

  updown = (LINES > cl_height) ? 1 : 0;
  leftright = (COLS > cl_width) ? 1 : 0;

  cl  = newwin(cl_height, cl_width, y, x);
  cld = newwin(cl_height, cl_width, y, x);

  h1 = subwin(cl, cl_height, di_width, starty, startx); startx += di_width;
  h2 = subwin(cl, cl_height, di_width, starty, startx); startx += di_width;
  c1 = subwin(cl, cl_height, co_width, starty, startx); startx += co_width;
  m1 = subwin(cl, cl_height, di_width, starty, startx); startx += di_width;
  m2 = subwin(cl, cl_height, di_width, starty, startx); startx += di_width;
  c2 = subwin(cl, cl_height, co_width, starty, startx); startx += co_width;
  s1 = subwin(cl, cl_height, di_width, starty, startx); startx += di_width;
  s2 = subwin(cl, cl_height, di_width, starty, startx);

  curs_set(0);

  while (1) {
    time(&t_time);
    tm_time = localtime(&t_time);
    vtclock_print_string(h1, 0, 0, digits[tm_time->tm_hour / 10]);
    vtclock_print_string(h2, 0, 0, digits[tm_time->tm_hour % 10]);
    vtclock_print_string(m1, 0, 0, digits[tm_time->tm_min / 10]);
    vtclock_print_string(m2, 0, 0, digits[tm_time->tm_min % 10]);
    vtclock_print_string(s1, 0, 0, digits[tm_time->tm_sec / 10]);
    vtclock_print_string(s2, 0, 0, digits[tm_time->tm_sec % 10]);
    vtclock_print_string(c1, 0, 0, colon);
    vtclock_print_string(c2, 0, 0, colon); 

    if (waitfor >= VTCLOCK_BOUNCE) {
      /* erase old */
      mvwin(cld, y, x);
      wrefresh(cld);

      /* bouncy bouncy */
      futurex = x + leftright;
      futurey = y + updown;
      if ((futurex < 0) || (futurex > (COLS - cl_width))) {
	futurex = x + (leftright *= -1);
      }
      if ((futurey < 0) || (futurey > (LINES - cl_height))) {
	futurey = y + (updown *= -1);
      }
      x = futurex;
      y = futurey;

      waitfor = 0;
    }

    mvwin(cl,y,x);
    wrefresh(cl);
    mydelay();
    ++waitfor;
  }
 
  endwin();
  return 0;
}

