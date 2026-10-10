#ifndef UINPUT_THING_H
#define UINPUT_THING_H

int init_uinput();
int close_uinput(int fd);
int switch_tablet_mode(int fd, int value);

#endif