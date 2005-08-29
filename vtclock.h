#ifndef VTCLOCK__VTCLOCK_H
#define VTCLOCK__VTCLOCK_H

#include "font.h"

typedef struct {
  vtclock_font *hour;
  vtclock_font *minute;
  vtclock_font *second;
  vtclock_font *colon1;
  vtclock_font *colon2;
} vtclock_config;

#endif /* VTCLOCK__VTCLOCK_H */
