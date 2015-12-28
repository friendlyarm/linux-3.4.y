#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>

typedef unsigned int u32;
typedef unsigned long long u64;
typedef int s32;

#include <linux/vr/vr_utgard_ioctl.h>
#include <linux/vr/vr_utgard_uk_types.h>

int main(int argc, char *argv[])
{
    int fd;
    unsigned int num_pages;
    unsigned int *page_ptr;
    unsigned int phys_addr;
    int i;
    int ret;
    _vr_uk_query_mmu_page_table_dump_size_s mmu_size_data = {0,};
    _vr_uk_dump_mmu_page_table_s mmu_info_data = {0,};

    fd = open("/dev/vr", O_RDWR);
    if (fd < 0) {
        fprintf(stderr, "can't open VR driver\n");
        return -1;
    }

    if(-1 != ioctl(fd, VR_IOC_MEM_QUERY_MMU_PAGE_TABLE_DUMP_SIZE, &mmu_size_data)) {
        mmu_info_data.buffer = (void*)malloc(mmu_size_data.size);
        mmu_info_data.size = mmu_size_data.size;
        if(-1 != ioctl(fd, VR_IOC_MEM_DUMP_MMU_PAGE_TABLE, &mmu_info_data)) {
            num_pages = mmu_info_data.page_table_dump_size / (4096 + 4);
            page_ptr = mmu_info_data.page_table_dump;
            for (i = 0; i < num_pages; i++) {
                phys_addr = *page_ptr;
                page_ptr++;
                printf(" ===> [MMU] addr(0x%x)\n", phys_addr );
                page_ptr += 4096 / sizeof(*page_ptr);
            }
        } else {
            fprintf(stderr, " ===> [MMU] ioctl dump mmu error, fd(%d)\n", fd );
        }
        free(mmu_info_data.buffer);
    } else {
        fprintf(stderr, " ===> [MMU] ioctl get size error, fd(%d)\n", fd );
    }

    close(fd);
    return 0;
}
