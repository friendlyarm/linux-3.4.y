#
# Copyright 2009 Nexell Co.
#
# Makefile for the S5P6818

obj-$(CONFIG_ARCH_CPU_SLSI) := cpu.o s5p6818.o gic.o smp_wfi.o timer.o irq.o clock.o devices.o cpufreq.o busfreq.o

obj-$(CONFIG_ARCH_CPU_SLSI)			+= soc/
obj-$(CONFIG_HOTPLUG_CPU)			+= hotplug.o
obj-$(CONFIG_SMP)					+= platsmp.o headsmp.o
obj-$(CONFIG_PM)					+= pm.o lldebug.o
obj-$(CONFIG_MMC_DW)				+= dev-mmc.o
obj-$(CONFIG_HAVE_PWM)				+= pwm.o
obj-$(CONFIG_CMA)					+= dev-cma.o
obj-$(CONFIG_ANDROID_RAM_CONSOLE)	+= dev-ramcon.o

#===============================================================================
# Support prototype
#===============================================================================
PROTOTYPE := prototype
MODULES	  := module
BASEDIR	  := base

include $(srctree)/arch/arm/mach-s5p6818/$(PROTOTYPE).mk

KBUILD_CFLAGS += -I$(srctree)/$(MACHINE)$(PROTOTYPE)/$(BASEDIR)	\
				 -I$(srctree)/$(MACHINE)$(PROTOTYPE)/$(MODULES)	\
				 -I$(srctree)/$(MACHINE)$(PROTOTYPE)/$(MODULES)

ifeq ($(CONFIG_S5P6818_PROTO_RELEASE),y)
KBUILD_CFLAGS += -D__LINUX__ -D__PRINTK__ -DNX_RELEASE
else
KBUILD_CFLAGS += -D__LINUX__ -D__PRINTK__ -DNX_DEBUG
endif

#===============================================================================
#	Build options for nexell soc
#===============================================================================
KBUILD_CFLAGS += -mfpu=vfp -mfloat-abi=softfp
