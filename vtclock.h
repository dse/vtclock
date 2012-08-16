#ifndef VTCLOCK__VTCLOCK_H
#define VTCLOCK__VTCLOCK_H

#define VTCLOCK_VERSION_MAJOR      0
#define VTCLOCK_VERSION_MINOR      0
#define VTCLOCK_VERSION_PATCHLEVEL 20120815

#define VTCLOCK_VERSION            "0.0.20120815"
#define VTCLOCK_COPYRIGHT_DATE     "2012"
#define VTCLOCK_COPYRIGHT_HOLDER   "Darren Embry"

#include "font.h"

typedef struct {
  vtclock_font *hour;
  vtclock_font *minute;
  vtclock_font *second;
  vtclock_font *colon1;
  vtclock_font *colon2;
} vtclock_config;

#endif /* VTCLOCK__VTCLOCK_H */
