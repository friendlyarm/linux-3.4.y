###############################################################################
#
# Siano Mobile Silicon, Inc.
# MDTV receiver kernel modules.
# Copyright (C) 2006-2008, Uri Shkolnik
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 2 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
###############################################################################


# Local variables initialization
SMS_HOSTLIB_SUBSYS := 0

SMS_USB_DRV := 0
SMS_SDIO_DRV := 0
SMS_SPI_DRV := 0
SMS_I2C_DRV := 0

# Default object, include in every build variant
SMSOBJ := smscoreapi.o sms-cards.o smsendian.o


EXTRA_CFLAGS += $(extra-cflags-y) $(extra-cflags-m)


# Kernel subsystems support

EXTRA_CFLAGS += -Idrivers/media/dvb/dvb-core

ifdef CONFIG_SMS_RC_SUPPORT_SUBSYS
ifneq ($(CONFIG_SMS_RC_SUPPORT_SUBSYS),n)
EXTRA_CFLAGS += -DSMS_RC_SUPPORT_SUBSYS
SMSOBJ += smsir.o
endif
endif

ifdef CONFIG_SMS_DVB_API_SUBSYS
ifneq ($(CONFIG_SMS_DVB_API_SUBSYS),n)
EXTRA_CFLAGS += -DSMS_DVB3_SUBSYS
SMSOBJ += smsdvb.o
endif
endif

ifdef CONFIG_SMS_HOSTLIB_SUBSYS
ifneq ($(CONFIG_SMS_HOSTLIB_SUBSYS),n)
SMS_HOSTLIB_SUBSYS := 1
EXTRA_CFLAGS += -DSMS_HOSTLIB_SUBSYS
SMSOBJ += smschar.o
endif
endif

ifdef CONFIG_SMS_NET_SUBSYS
ifneq ($(CONFIG_SMS_NET_SUBSYS),n)
SMS_NET_SUBSYS := 1
EXTRA_CFLAGS += -DSMS_NET_SUBSYS
SMSOBJ += smsnet.o
endif
endif

# Hardware (host) interfaces support
ifdef CONFIG_SMS_USB_DRV
ifneq ($(CONFIG_SMS_USB_DRV),n)
SMS_USB_DRV := 1
EXTRA_CFLAGS += -DSMS_USB_DRV
SMSOBJ += smsusb.o
endif
endif

ifdef CONFIG_SMS_SDIO_DRV
ifneq ($(CONFIG_SMS_SDIO_DRV),n)
SMS_SDIO_DRV := 1
EXTRA_CFLAGS += -DSMS_SDIO_DRV
SMSOBJ += smssdio.o
endif
endif

ifdef CONFIG_SMS_SPI_DRV
ifneq ($(CONFIG_SMS_SPI_DRV),n)
SMS_SPI_RV := 1
EXTRA_CFLAGS += -DSMS_SPI_DRV
#EXTRA_CFLAGS += -DSPIBUS_DEBUG
SMSOBJ += smsspidrv.o smsspicommon.o smsspiphy.o
endif
endif

ifdef CONFIG_SMS_I2C_DRV
ifneq ($(CONFIG_SMS_I2C_DRV),n)
SMS_I2C_DRV := 1
EXTRA_CFLAGS += -DSMS_I2C_DRV
SMSOBJ += smsi2c.o
endif
endif

# All selected in one module named smsmdtv
smsmdtv-objs := $(SMSOBJ)

obj-$(CONFIG_SMS_SIANO_MDTV) += smsmdtv.o

