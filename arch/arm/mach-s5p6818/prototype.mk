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

obj-y += $(PROTOTYPE)/$(BASEDIR)/nx_bit_accessor.o

obj-y += \
		$(PROTOTYPE)/$(MODULES)/nx_clkpwr.o		\
   		$(PROTOTYPE)/$(MODULES)/nx_rstcon.o		\
   		$(PROTOTYPE)/$(MODULES)/nx_tieoff.o		\
		$(PROTOTYPE)/$(MODULES)/nx_mcus.o		\
		$(PROTOTYPE)/$(MODULES)/nx_timer.o		\
		$(PROTOTYPE)/$(MODULES)/nx_gpio.o		\
		$(PROTOTYPE)/$(MODULES)/nx_alive.o		\
		$(PROTOTYPE)/$(MODULES)/nx_clkgen.o		\
		$(PROTOTYPE)/$(MODULES)/nx_ecid.o		\
		$(PROTOTYPE)/$(MODULES)/nx_wdt.o

obj-$(CONFIG_NXP_DISPLAY)				+= 	$(PROTOTYPE)/$(MODULES)/nx_displaytop.o		\
								   			$(PROTOTYPE)/$(MODULES)/nx_disptop_clkgen.o	\
											$(PROTOTYPE)/$(MODULES)/nx_dualdisplay.o	\
								   			$(PROTOTYPE)/$(MODULES)/nx_mlc.o			\
								   			$(PROTOTYPE)/$(MODULES)/nx_dpc.o

obj-$(CONFIG_NXP_DISPLAY_LVDS)			+= 	$(PROTOTYPE)/$(MODULES)/nx_lvds.o
obj-$(CONFIG_NXP_DISPLAY_RESCONV)		+= 	$(PROTOTYPE)/$(MODULES)/nx_resconv.o
obj-$(CONFIG_NXP_DISPLAY_HDMI)			+= 	$(PROTOTYPE)/$(MODULES)/nx_hdmi.o
obj-$(CONFIG_NXP_DISPLAY_HDMI)			+= 	$(PROTOTYPE)/$(MODULES)/nx_ecid.o
obj-$(CONFIG_NXP_DISPLAY_MIPI)			+=  $(PROTOTYPE)/$(MODULES)/nx_mipi.o

obj-$(CONFIG_RTC_DRV_NXP)				+= 	$(PROTOTYPE)/$(MODULES)/nx_rtc.o

obj-$(CONFIG_HAVE_PWM)					+= 	$(PROTOTYPE)/$(MODULES)/nx_pwm.o
obj-$(CONFIG_NXP_ADC)					+= 	$(PROTOTYPE)/$(MODULES)/nx_adc.o
obj-$(CONFIG_VIDEO_NXP_CAPTURE)			+=  $(PROTOTYPE)/$(MODULES)/nx_vip.o
obj-$(CONFIG_SLSIAP_BACKWARD_CAMERA)	+=  $(PROTOTYPE)/$(MODULES)/nx_vip.o
obj-$(CONFIG_NXP_OUT_HDMI)				+= 	$(PROTOTYPE)/$(MODULES)/nx_hdmi.o
obj-$(CONFIG_NXP_OUT_HDMI)				+= 	$(PROTOTYPE)/$(MODULES)/nx_ecid.o

obj-$(CONFIG_NXP_M2M_SCALER)			+=  $(PROTOTYPE)/$(MODULES)/nx_scaler.o
obj-$(CONFIG_NXP_CAPTURE_MIPI_CSI)		+=  $(PROTOTYPE)/$(MODULES)/nx_mipi.o
obj-$(CONFIG_NXP_MP2TS_IF)				+=  $(PROTOTYPE)/$(MODULES)/nx_mpegtsi.o

obj-$(CONFIG_RTC_DRV_NXP)				+=  $(PROTOTYPE)/$(MODULES)/nx_rtc.o

obj-$(CONFIG_PPM_NXP) 					+=  $(PROTOTYPE)/$(MODULES)/nx_ppm.o
obj-$(CONFIG_SND_NXP_PDM) 				+=  $(PROTOTYPE)/$(MODULES)/nx_pdm.o
obj-$(CONFIG_SENSORS_NXP_TMU) 			+=  $(PROTOTYPE)/$(MODULES)/nx_tmu.o
obj-$(CONFIG_NXP_DISPLAY_TVOUT)			+= 	$(PROTOTYPE)/$(MODULES)/nx_hdmi.o
obj-$(CONFIG_NXP_M2M_DEINTERLACER) +=  $(PROTOTYPE)/$(MODULES)/nx_deinterlace.o
