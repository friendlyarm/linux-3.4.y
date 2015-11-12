#
# Copyright 2009 Nexell Co.
#
# Makefile for the S5P4418

obj-$(CONFIG_ARCH_S5P4418) := cpu.o s5p4418.o timer.o irq.o clock.o	devices.o

obj-$(CONFIG_ARCH_S5P4418) 			+= soc/
obj-$(CONFIG_HOTPLUG_CPU)			+= hotplug.o
obj-$(CONFIG_SMP)					+= platsmp.o headsmp.o
obj-$(CONFIG_PM)					+= pm.o lldebug.o
obj-$(CONFIG_ARM_NXP_CPUFREQ)   += cpufreq.o cpufreq-limit.o
obj-$(CONFIG_ARM_NXP_CPUFREQ_BY_RESOURCE)   += cpufreq_resource.o
obj-$(CONFIG_MMC_DW)				+= dev-mmc.o
obj-$(CONFIG_HAVE_PWM)				+= pwm.o
obj-$(CONFIG_CMA)					+= cma_reserve_mem.o
obj-$(CONFIG_ANDROID_RAM_CONSOLE)	+= ram_con_dev.o
obj-y								+= nxp-dfs-bclk.o

#===============================================================================
# Support prototype
#===============================================================================
PROTOTYPE := prototype
MODULES	  := module
BASEDIR	  := base

include $(srctree)/arch/arm/mach-s5p4418/$(PROTOTYPE).mk

KBUILD_CFLAGS += -I$(srctree)/$(MACHINE)$(PROTOTYPE)/$(BASEDIR)	\
				 -I$(srctree)/$(MACHINE)$(PROTOTYPE)/$(MODULES)	\
				 -I$(srctree)/$(MACHINE)$(PROTOTYPE)/$(MODULES)

ifeq ($(CONFIG_S5P4418_PROTO_RELEASE),y)
KBUILD_CFLAGS += -D__LINUX__ -D__PRINTK__ -DNX_RELEASE
else
KBUILD_CFLAGS += -D__LINUX__ -D__PRINTK__ -DNX_DEBUG
endif

#===============================================================================
#	Build options for nexell soc
#===============================================================================
KBUILD_CFLAGS += -mtune=cortex-a9 -mfpu=vfp -mfloat-abi=softfp
# -mfpu=vfpv3
