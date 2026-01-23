#include <linux/input.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdio.h>
#include <string.h>
#include <sys/ioctl.h>

#define BITS_PER_LONG   (sizeof(unsigned long) * 8)
#define NBITS(x)        ((((x) - 1) / BITS_PER_LONG) + 1)
#define TEST_BIT(bit, array) \
    ((array[(bit) / BITS_PER_LONG] >> ((bit) % BITS_PER_LONG)) & 1)

int is_mouse(int fd) {
    unsigned long evbit[NBITS(EV_MAX)];
    unsigned long relbit[NBITS(REL_MAX)];
    unsigned long keybit[NBITS(KEY_MAX)];

    memset(evbit, 0, sizeof(evbit));
    memset(relbit, 0, sizeof(relbit));
    memset(keybit, 0, sizeof(keybit));

    if (ioctl(fd, EVIOCGBIT(0, sizeof(evbit)), evbit) < 0)
        return 0;

    if (!TEST_BIT(EV_REL, evbit))
        return 0;

    ioctl(fd, EVIOCGBIT(EV_REL, sizeof(relbit)), relbit);
    ioctl(fd, EVIOCGBIT(EV_KEY, sizeof(keybit)), keybit);

    return TEST_BIT(REL_X, relbit) &&
           TEST_BIT(REL_Y, relbit) &&
           TEST_BIT(BTN_LEFT, keybit);
}

int main(void) {
    char path[64];

    for (int i = 0; i < 32; i++) {
        snprintf(path, sizeof(path), "/dev/input/event%d", i);

        int fd = open(path, O_RDONLY);
        if (fd < 0)
            continue;

        if (is_mouse(fd))
            printf("Mouse: %s\n", path);

        close(fd);
    }
    return 0;
}
