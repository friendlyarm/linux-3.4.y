TARGET			= nx_vpu
obj-$(CONFIG_NXP_VPU)	:= $(TARGET).o

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

######################################################################

EXTRA_CFLAGS += -I./include
EXTRA_CFLAGS += -I./firmware
EXTRA_CFLAGS += -I../libnxvmalloc

######################################################################