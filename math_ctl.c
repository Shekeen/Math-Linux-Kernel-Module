#include <err.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <limits.h>

#include "math.h"

const char *cmd_name(int cmd)
{
    switch (cmd) {
        case MATH_IOCTL_SQR:
                return "MATH_IOCTL_SQR";

        case MATH_IOCTL_NEG:
                return "MATH_IOCTL_NEG";

        case MATH_IOCTL_ADD:
                return "MATH_IOCTL_ADD";

        case MATH_IOCTL_SUB:
                return "MATH_IOCTL_SUB";

        case MATH_IOCTL_MUL:
                return "MATH_IOCTL_MUL";

        case MATH_IOCTL_DIV:
                return "MATH_IOCTL_DIV";
        default:
                return "UNKNOWN IOCTL";
    }
}

void try_open_many_files(void)
{
    int fd[5], i;

    for (i=0; i<4; i++) {
        fd[i]  = open("/dev/math", O_RDWR);
        if (fd[i] < 0)
            err(1, "open(%d)", i);
    }

    fd[5] = open("/dev/math", O_RDWR);
    if (fd[5] >= 0)
        err(1, "open(5) not failed, but has to fail!");
}

void try_ioctl_1(int fd, unsigned int cmd, int arg1, int should_succeed, int result)
{
    int x[2];
    int ret;

    x[0] = arg1;

    ret = ioctl(fd, cmd, &x);
    if (!should_succeed) {
        if (ret >= 0)
            errx(3, "ioctl %s(%d) should fail, but returned %d.\n",
                    cmd_name(cmd), arg1, ret);
        return;
    }

    if (ret < 0)
        err(2, "ioctl %s(%d) should return %d, but returned error:",
                cmd_name(cmd), arg1, result);
    if (x[1] != result)
        errx(4, "ioctl %s(%d) should return %d, but returned %d.\n",
                cmd_name(cmd), arg1, result, x[2]);
}

void try_ioctl_2(int fd, unsigned int cmd, int arg1, int arg2, int should_succeed, int result)
{
    int x[3];
    int ret;

    x[0] = arg1;
    x[1] = arg2;

    ret = ioctl(fd, cmd, &x);
    if (!should_succeed) {
        if (ret >= 0)
            errx(3, "ioctl %s(%d, %d) should fail, but returned %d.\n",
                    cmd_name(cmd), arg1, arg2, ret);
        return;
    }

    if (ret < 0)
        err(2, "ioctl %s(%d, %d) should return %d, but returned error:",
                cmd_name(cmd), arg1, arg2, result);
    if (x[2] != result)
        errx(4, "ioctl %s(%d, %d) should return %d, but returned %d.\n",
                cmd_name(cmd), arg1, arg2, result, x[2]);
}

int main(int argc, char *argv[])
{
    int fd;
   
    /* Try to open math character device */
    fd  = open("/dev/math", O_RDWR);
    if (fd < 0)
        err(1, "Cannot open 'math' character device.\n"
               "Probably you should create a device node with \"mknod /dev/math c X 0\""
               " where X is a major number of the device.\n"
               "(You can identify the major number in /proc/devices.)\n"
			   "If the error is \"Permission denied\" then you probably should run"
			   " math_ctl as root.\n"
               "open");


    try_ioctl_1(fd, MATH_IOCTL_SQR, 2, 1, 4);
    /* Try MATH_GET_SQR overflow */
    try_ioctl_1(fd, MATH_IOCTL_SQR, INT_MAX/2, 0, 0);

    try_ioctl_1(fd, MATH_IOCTL_NEG, 4, 1, -4);
    /* In C you cannot do -x for all integer x.
     * If x is INT_MIN, -x overflows. */
    try_ioctl_1(fd, MATH_IOCTL_NEG, INT_MIN, 0, 0);

    try_ioctl_2(fd, MATH_IOCTL_ADD, 2, 2, 1, 4);
    try_ioctl_2(fd, MATH_IOCTL_ADD, 2, -5, 1, -3);
    /* overflow */
    try_ioctl_2(fd, MATH_IOCTL_ADD, INT_MAX, 2, 0, 0);

    try_ioctl_2(fd, MATH_IOCTL_SUB, 2, 2, 1, 0);
    try_ioctl_2(fd, MATH_IOCTL_SUB, 2, -2, 1, 4);

    try_ioctl_2(fd, MATH_IOCTL_MUL, 2, 3, 1, 6);

    try_ioctl_2(fd, MATH_IOCTL_DIV, 6, 3, 1, 2);
    try_ioctl_2(fd, MATH_IOCTL_DIV, 200, -3, 1, -66);
    /* Divide by zero */
    try_ioctl_2(fd, MATH_IOCTL_DIV, 1, 0, 0, 0);

    /* Try to open 'math' many times */
    close(fd);
    try_open_many_files();

    puts("All tests are passed.");
    return 0;
}
