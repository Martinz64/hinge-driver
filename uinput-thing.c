
#include <stdio.h>
#include <unistd.h>
#include <time.h>
#include <fcntl.h>
#include <stdlib.h>
#include <string.h>

#include <linux/uinput.h>
#include <linux/input.h>

#include "uinput-thing.h"


int init_uinput(){
    struct uinput_user_dev uidev;
    int fd = open("/dev/uinput", O_WRONLY | O_NONBLOCK);
    if(fd < 0){
        perror("error: open");
        return -1;
    }

    if(ioctl(fd, UI_SET_EVBIT, EV_SW) < 0){
        perror("error: ioctl");
        return -1;
    }
    if(ioctl(fd, UI_SET_SWBIT, SW_TABLET_MODE) < 0){
        perror("error: ioctl");
        return -1;
    }

    memset(&uidev, 0, sizeof(uidev));
    snprintf(uidev.name, UINPUT_MAX_NAME_SIZE, "Virtual tablet mode switch");
    uidev.id.bustype = BUS_USB;
    uidev.id.vendor  = 0x1;
    uidev.id.product = 0x1;
    uidev.id.version = 1;

    if(write(fd, &uidev, sizeof(uidev)) < 0){
        perror("error: write");
        return -1;
    }
    if(ioctl(fd, UI_DEV_CREATE) < 0){
        perror("error: ioctl");
        return -1;
    }
    return fd;
}

int close_uinput(int fd){
    if(ioctl(fd, UI_DEV_DESTROY) < 0){
        perror("error: ioctl");
        return -1;
    }

    close(fd);
    return 0;
}

int switch_tablet_mode(int fd, int value){
    struct input_event     ev;
    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SW;
    ev.code = SW_TABLET_MODE;
    ev.value = value;
    if(write(fd, &ev, sizeof(struct input_event)) < 0){
        perror("error: write");
        return -1;
    }

    memset(&ev, 0, sizeof(struct input_event));
    ev.type = EV_SYN;
    ev.code = 0;
    ev.value = 0;
    if(write(fd, &ev, sizeof(struct input_event)) < 0){
        perror("error: write");
        return -1;
    }
    return 0;
}

/*int main(){

    int fd = init_uinput();
    if(fd < 0){
        printf("Err\n");
    }
    sleep(1);
    switch_tablet_mode(fd,1);
    sleep(3);
    switch_tablet_mode(fd,0);
    sleep(1);
    close_uinput(fd);
}*/