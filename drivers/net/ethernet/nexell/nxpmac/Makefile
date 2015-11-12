obj-$(CONFIG_NXPMAC_ETH) += nxpmac.o
nxpmac-$(CONFIG_NXPMAC_PLATFORM) += nxpmac_platform.o
nxpmac-$(CONFIG_NXPMAC_PCI) += nxpmac_pci.o
nxpmac-objs:= nxpmac_main.o nxpmac_ethtool.o nxpmac_mdio.o ring_mode.o \
	      chain_mode.o dwmac_lib.o dwmac1000_core.o  dwmac1000_dma.o \
	      dwmac100_core.o dwmac100_dma.o enh_desc.o  norm_desc.o \
	      mmc_core.o nxpmac_hwtstamp.o nxpmac_ptp.o $(nxpmac-y)
