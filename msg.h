#ifndef VTCLOCK__MSG_H
#define VTCLOCK__MSG_H

void init_message (int cols, int argc, char **argv, int is_pipe, int delay);
char *get_next_message (void);

#endif /* VTCLOCK__MSG_H */
