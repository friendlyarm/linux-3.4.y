
obj-$(CONFIG_MTD_NAND_NXP)	+= nand.o

ifeq ($(CONFIG_MTD_NAND_MANUAL_ELP_PATH),y)
obj-$(CONFIG_MTD_NAND_ECC_HW)	+= nand_ecc_elp.o
else
obj-$(CONFIG_MTD_NAND_ECC_HW)	+= nand_ecc.o
endif

ifdef CONFIG_ARM64
obj-y += mtd.lib64.o
else
obj-y += mtd.lib.o
endif
