#ifndef FIGLET_C
#define FIGLET_C

typedef struct figlet_options {
  char *font_name;
} figlet_options;

vtclock_font *
generate_figlet_font (figlet_options *);

vtclock_config *
generate_figlet_config (figlet_options *);

#endif /* FIGLET_C */
