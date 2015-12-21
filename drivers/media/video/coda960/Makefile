TARGET			= nx_vpu

KDIR			:= ../../../../kernel/kernel-3.4.39
#STRIP_CMD		= arm-cortex_a9-linux-gnueabi-strip

obj-m			:= $(TARGET).o

$(TARGET)-objs	+=

######################################################################
# Source Cods
$(TARGET)-objs	+=

# Driver Main
$(TARGET)-objs	+= linux/linux_vpu_drv.o

# OS APIS
$(TARGET)-objs	+= osapi/drv_osapi_linux.o

# VPU Control Sources
$(TARGET)-objs	+= src/vpu_hw_interface.o
$(TARGET)-objs	+= src/nx_jpu_api.o
$(TARGET)-objs	+= src/nx_vpu_api.o
$(TARGET)-objs	+= src/nx_vpu_gdi.o


################################################################################
#	Build options for board type
################################################################################
PWD     := $(shell pwd)
KBUILD_CFLAGS += -I./include
KBUILD_CFLAGS += -I./firmware
KBUILD_CFLAGS += -I../libnxvmalloc

default:
	$(MAKE) -C $(KDIR) M=$(PWD) modules

clean:
	@find . -type f \( -name '*.ko' -o -name '*.mod.*'		\
					-o -name '*.o' -o -name '.*.cmd'		\
					-o -name '*.order' -o -name '*.symvers'	\
					-o -name '*.' \) -print | xargs rm -f
	rm -rf .tmp_versions

