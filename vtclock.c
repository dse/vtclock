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

int
main() {
  WINDOW *h1, *h2, *m1, *m2, *s1, *s2, *c1, *c2;
  int startx, starty, di_width, cl_height, co_width, cl_width;
  int i;
  struct tm *tm_time;
  time_t t_time;

  initscr();

  cl_height = DIGIT_HEIGHT;
  di_width = DIGIT_WIDTH;
  co_width = COLON_WIDTH;
  cl_width = co_width * 2 + di_width * 6;
  
  starty = (LINES - cl_height) / 2;
  startx = (COLS - cl_width) / 2;

  h1 = newwin(cl_height, di_width, starty, startx); startx += di_width;
  h2 = newwin(cl_height, di_width, starty, startx); startx += di_width;
  c1 = newwin(cl_height, co_width, starty, startx); startx += co_width;
  m1 = newwin(cl_height, di_width, starty, startx); startx += di_width;
  m2 = newwin(cl_height, di_width, starty, startx); startx += di_width;
  c2 = newwin(cl_height, co_width, starty, startx); startx += co_width;
  s1 = newwin(cl_height, di_width, starty, startx); startx += di_width;
  s2 = newwin(cl_height, di_width, starty, startx);

  while (1) {
    time(&t_time);
    tm_time = localtime(&t_time);
    mvwprintw(h1, 0, 0, digits[tm_time->tm_hour / 10]);
    mvwprintw(h2, 0, 0, digits[tm_time->tm_hour % 10]);
    mvwprintw(m1, 0, 0, digits[tm_time->tm_min / 10]);
    mvwprintw(m2, 0, 0, digits[tm_time->tm_min % 10]);
    mvwprintw(s1, 0, 0, digits[tm_time->tm_sec / 10]);
    mvwprintw(s2, 0, 0, digits[tm_time->tm_sec % 10]);
    mvwprintw(c1, 0, 0, colon);
    mvwprintw(c2, 0, 0, colon); 
    wrefresh(h1);
    wrefresh(h2);
    wrefresh(c1);
    wrefresh(m1);
    wrefresh(m2);
    wrefresh(c2);
    wrefresh(s1);
    wrefresh(s2);
    mydelay();
  }
 
  endwin();
  return 0;
}

