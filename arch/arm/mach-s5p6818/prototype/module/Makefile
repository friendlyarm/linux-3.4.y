#
# (C) Copyright 2009
# jung hyun kim, Nexell Co, <jhkim@nexell.co.kr>
#
# See file CREDITS for list of people who contributed to this
# project.
#
# This program is free software; you can redistribute it and/or
# modify it under the terms of the GNU General Public License as
# published by the Free Software Foundation; either version 2 of
# the License, or (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston,
# MA 02111-1307 USA
#

obj-y += \
		nx_clkpwr.o \
		nx_rstcon.o \
		nx_tieoff.o \
		nx_mcus.o \
		nx_timer.o \
		nx_gpio.o \
		nx_alive.o \
		nx_clkgen.o \
		nx_ecid.o \
		nx_wdt.o

obj-$(CONFIG_NXP_DISPLAY) += \
		nx_displaytop.o \
		nx_disptop_clkgen.o \
		nx_dualdisplay.o \
		nx_mlc.o \
		nx_dpc.o

obj-$(CONFIG_NXP_DISPLAY_LVDS)			+= nx_lvds.o
obj-$(CONFIG_NXP_DISPLAY_RESCONV)		+= nx_resconv.o
obj-$(CONFIG_NXP_DISPLAY_HDMI)			+= nx_hdmi.o
obj-$(CONFIG_NXP_DISPLAY_HDMI)			+= nx_ecid.o
obj-$(CONFIG_NXP_DISPLAY_MIPI)			+= nx_mipi.o

obj-$(CONFIG_RTC_DRV_NXP)				+= nx_rtc.o

obj-$(CONFIG_HAVE_PWM)					+= nx_pwm.o
obj-$(CONFIG_NXP_ADC)					+= nx_adc.o
obj-$(CONFIG_VIDEO_NXP_CAPTURE)			+= nx_vip.o
obj-$(CONFIG_SLSIAP_BACKWARD_CAMERA)	+= nx_vip.o
obj-$(CONFIG_NXP_OUT_HDMI)				+= nx_hdmi.o
obj-$(CONFIG_NXP_OUT_HDMI)				+= nx_ecid.o

obj-$(CONFIG_NXP_M2M_SCALER)			+= nx_scaler.o
obj-$(CONFIG_NXP_CAPTURE_MIPI_CSI)		+= nx_mipi.o
obj-$(CONFIG_NXP_MP2TS_IF)				+= nx_mpegtsi.o

obj-$(CONFIG_RTC_DRV_NXP)				+= nx_rtc.o

obj-$(CONFIG_PPM_NXP)					+= nx_ppm.o
obj-$(CONFIG_SND_NXP_PDM)				+= nx_pdm.o
obj-$(CONFIG_SENSORS_NXP_TMU)			+= nx_tmu.o
obj-$(CONFIG_NXP_DISPLAY_TVOUT)			+= nx_hdmi.o
obj-$(CONFIG_NXP_M2M_DEINTERLACER)		+= nx_deinterlace.o
