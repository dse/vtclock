#ifndef VTCLOCK__FONT_H
#define VTCLOCK__FONT_H

typedef struct {
  int digit_height;
  int digit_width;
  int colon_width;
  char *digits[10];
  char *colon;
} vtclock_font;

#endif /* VTCLOCK__FONT_H */

