#
# Copyright 2009 Nexell Co.
#
# Makefile for the Nexell platform board

ifeq ($(CONFIG_PLAT_S5P6818_VTK),y)
PLATBOARD :=vtk
endif

ifeq ($(CONFIG_PLAT_S5P6818_DRONE),y)
PLATBOARD :=drone
endif

ifeq ($(CONFIG_PLAT_S5P6818_FDONE),y)
PLATBOARD :=FDONE
endif

ifeq ($(CONFIG_PLAT_S5P6818_SVT),y)
PLATBOARD :=svt
endif

ifeq ($(CONFIG_PLAT_S5P6818_ASB),y)
PLATBOARD :=asb
endif

ifeq ($(CONFIG_PLAT_S5P6818_NANOPI3),y)
PLATBOARD := nanopi3
endif

#
# Build options
#

obj-y	:= $(PLATBOARD)/

KBUILD_AFLAGS 	+= -I$(srctree)/$(platdirs)common 	\
				   -I$(srctree)/$(platdirs)$(PLATBOARD)/include
KBUILD_CFLAGS 	+= -I$(srctree)/$(platdirs)common	\
				   -I$(srctree)/$(platdirs)$(PLATBOARD)/include
KBUILD_CPPFLAGS += -I$(srctree)/$(platdirs)common	\
				   -I$(srctree)/$(platdirs)$(PLATBOARD)/include

