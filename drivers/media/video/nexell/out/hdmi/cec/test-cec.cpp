/**
 * This is a simple NEXELL HDMI CEC user test program.
 * compile : CROSS_COMPILER test-cec.cpp -o test-cec
 * command : test_cec
 */
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>

#ifndef _IO
#define	TYPE_SHIFT		8
#define _IO(type,nr)	((type<<TYPE_SHIFT) | nr)
#define _IOC_TYPE(nr)	((nr>>TYPE_SHIFT) & 0xFF)
#define _IOC_NR(nr)		(nr & 0xFF)
#endif

/*
 * 	_IOC_TYPE = 0x78 : 	IOC_NX_MAGIC & 0xFF
 */
#define	IOC_NX_MAGIC	0x6e78	/* "nx" */

enum {
    IOCTL_HDMI_CEC_SETLADDR    =  _IOW(IOC_NX_MAGIC, 1, unsigned int),
};

static void dump_buf(uint8_t *buf, int read_size)
{
    printf("================================>\n");
    int i;
    for (i = 0; i < read_size; i++) {
        printf("0x%x, ", buf[i]);
    }
    printf("\n<================================\n");
}

int main(int argc, char *argv[])
{
    int cec_fd = open("/dev/hdmi-cec", O_RDWR);
    if(cec_fd < 0) {
         fprintf(stderr, "error open /dev/hdmi-cec\n");
         return -1;
    }

    uint8_t buf[16];

    while (1) {
        ssize_t read_size = read(cec_fd, buf, 16);
        if (read_size <= 0) {
            fprintf(stderr, "error read size %ld\n", read_size);
            close(cec_fd);
            return -1;
        }
        dump_buf(buf, read_size);
    }

    close(cec_fd);
    return 0;
}
