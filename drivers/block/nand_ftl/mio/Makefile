KBUILD_CFLAGS += $(shell cp -f drivers/block/nand_ftl/mio/media/include/include.o.prebuilt drivers/block/nand_ftl/mio/media/include/include.o)
KBUILD_CFLAGS += $(shell cp -f drivers/block/nand_ftl/mio/media/ftl/ftl.o.prebuilt drivers/block/nand_ftl/mio/media/ftl/ftl.o)
KBUILD_CFLAGS += $(shell cp -f drivers/block/nand_ftl/mio/media/nfc/nfc.sche.o.prebuilt drivers/block/nand_ftl/mio/media/nfc/nfc.sche.o)

obj-$(CONFIG_NAND_FTL) := mio.o
mio-y += mio.block.o \
         mio.media.o \
         mio.sys.o \
         mio.smart.o \
         media/exchange.o \
         media/include/include.o \
         media/ftl/ftl.o \
         media/nfc/nfc.sche.o \
         media/nfc/phy/nfc.phy.o \
         media/nfc/phy/nfc.phy.rand.o \
         media/nfc/phy/nfc.phy.scan.o \
         media/nfc/phy/nfc.phy.readretry.o \
         media/nfc/phy/nfc.phy.lowapi.o \
         media/nfc/phy/nfc.phy.lowapi.rawread.o

ifdef CONFIG_ARM64
mio-y += media/nfc/phy/nfc.lib64.o
else
mio-y += media/nfc/phy/nfc.lib.o
endif
