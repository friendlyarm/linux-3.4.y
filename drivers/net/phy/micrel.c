/*
 * drivers/net/phy/micrel.c
 *
 * Driver for Micrel PHYs
 *
 * Author: David J. Choi
 *
 * Copyright (c) 2010 Micrel, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 * Support : Micrel Phy:
 *		Giga phys: ksz9021, ksz9031
 *		10/100 Phys : ks8001, ks8721, ks8737, ks8041
 *			   ks8021, ks8031, ks8051,
 *			   ks8081, ks8091, ks8061
 */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/phy.h>
#include <linux/micrel_phy.h>

/* general Interrupt control/status reg in vendor specific block. */
#define MII_KSZPHY_INTCS					0x1B
#define KSZPHY_INTCS_JABBER					(1 << 15)
#define KSZPHY_INTCS_RECEIVE_ERR			(1 << 14)
#define KSZPHY_INTCS_PAGE_RECEIVE			(1 << 13)
#define KSZPHY_INTCS_PARELLEL				(1 << 12)
#define KSZPHY_INTCS_LINK_PARTNER_ACK		(1 << 11)
#define KSZPHY_INTCS_LINK_DOWN				(1 << 10)
#define KSZPHY_INTCS_REMOTE_FAULT			(1 << 9)
#define KSZPHY_INTCS_LINK_UP				(1 << 8)
#define KSZPHY_INTCS_ALL					(KSZPHY_INTCS_LINK_UP |\
											KSZPHY_INTCS_LINK_DOWN)

/* general PHY control reg in vendor specific block. */
#define	MII_KSZPHY_CTRL			0x1F
/* bitmap of PHY register to set interrupt mode */
#define KSZPHY_CTRL_INT_ACTIVE_HIGH			(1 << 9)
#define KSZ9021_CTRL_INT_ACTIVE_HIGH		(1 << 14)
#define KS8737_CTRL_INT_ACTIVE_HIGH			(1 << 14)
#define KSZ8051_RMII_50MHZ_CLK				(1 << 7)

/* KSZ9021 Write/read to/from extended registers */
#define MII_KSZ9021_EXTREG					0x0b
#define KSZ9021_EXTREG_WRITE				0x8000

#define MII_KSZ9021_EXTREG_WRITE			0x0c
#define MII_KSZ9021_EXTREG_READ				0x0d

#define MII_KSZ9021_EXT_RGMII_RXC_SHIFT		12
#define MII_KSZ9021_EXT_RGMII_RX_DV_SHIFT	8
#define MII_KSZ9021_EXT_RGMII_TXC_SHIFT		4
#define MII_KSZ9021_EXT_RGMII_TX_EN_SHIFT	0

#define MII_KSZ90x1_EXT_RGMII_RX3_SHIFT		12
#define MII_KSZ90x1_EXT_RGMII_RX2_SHIFT		8
#define MII_KSZ90x1_EXT_RGMII_RX1_SHIFT		4
#define MII_KSZ90x1_EXT_RGMII_RX0_SHIFT		0

#define MII_KSZ90x1_EXT_RGMII_TX3_SHIFT		12
#define MII_KSZ90x1_EXT_RGMII_TX2_SHIFT		8
#define MII_KSZ90x1_EXT_RGMII_TX1_SHIFT		4
#define MII_KSZ90x1_EXT_RGMII_TX0_SHIFT		0

/* Extended registers */
#define MII_KSZ9021_EXT_RGMII_CLOCK_SKEW	0x104
#define MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW	0x105
#define MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW	0x106

/* KSZ9031 Write/read to/from extended registers */
/* Register operations */
#define MII_KSZ9031_MOD_REG					0x0000
/* Data operations */
#define MII_KSZ9031_MOD_DATA_NO_POST_INC	0x4000
#define MII_KSZ9031_MOD_DATA_POST_INC_RW	0x8000
#define MII_KSZ9031_MOD_DATA_POST_INC_W		0xC000

#define MII_KSZ9031_MMD_ACCES_CTRL			0x0d
#define MII_KSZ9031_MMD_REG_DATA			0x0e

/* MMD Address 2h registers */
#define MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW	0x4
#define MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW	0x5
#define MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW	0x6
#define MII_KSZ9031_EXT_RGMII_CLOCK_SKEW	0x8

#define MII_KSZ9031_EXT_RGMII_RX_DV_SHIFT	4
#define MII_KSZ9031_EXT_RGMII_TX_EN_SHIFT	0

#define MII_KSZ9031_EXT_RGMII_GTX_CLK_SHIFT	5
#define MII_KSZ9031_EXT_RGMII_RX_CLK_SHIFT	0

#define PS_TO_REG							200

static int ksz_config_flags(struct phy_device *phydev)
{
	int regval;

	if (phydev->dev_flags & MICREL_PHY_50MHZ_CLK) {
		regval = phy_read(phydev, MII_KSZPHY_CTRL);
		regval |= KSZ8051_RMII_50MHZ_CLK;
		return phy_write(phydev, MII_KSZPHY_CTRL, regval);
	}
	return 0;
}

static int ksz9021_phy_extended_write(struct phy_device *phydev,
								u32 regnum, u16 val)
{
	phy_write(phydev, MII_KSZ9021_EXTREG, KSZ9021_EXTREG_WRITE | regnum);
	return phy_write(phydev, MII_KSZ9021_EXTREG_WRITE, val);
}

static int ksz9021_phy_extended_read(struct phy_device *phydev,
								u32 regnum)
{
	phy_write(phydev, MII_KSZ9021_EXTREG, regnum);
	return phy_read(phydev, MII_KSZ9021_EXTREG_READ);
}

/* Accessors to extended registers*/
static int ksz9031_phy_extended_write(struct phy_device *phydev,
					int devaddr, int regnum, u16 mode, u16 val)
{
	/*select register addr for mmd*/
	phy_write(phydev, MII_KSZ9031_MMD_ACCES_CTRL,   devaddr);
	/*select register for mmd*/
	phy_write(phydev, MII_KSZ9031_MMD_REG_DATA,     regnum);
	/*setup mode*/
	phy_write(phydev, MII_KSZ9031_MMD_ACCES_CTRL,   (mode | devaddr));
	/*write the value*/
	return phy_write(phydev, MII_KSZ9031_MMD_REG_DATA, val);
}

static int ksz9031_phy_extended_read(struct phy_device *phydev, int devaddr,
					int regnum, u16 mode)
{
	phy_write(phydev, MII_KSZ9031_MMD_ACCES_CTRL,   devaddr);
	phy_write(phydev, MII_KSZ9031_MMD_REG_DATA,     regnum);
	phy_write(phydev, MII_KSZ9031_MMD_ACCES_CTRL,   (devaddr | mode));
	return phy_read(phydev, MII_KSZ9031_MMD_REG_DATA);
}

static int kszphy_ack_interrupt(struct phy_device *phydev)
{
	/* bit[7..0] int status, which is a read and clear register. */
	int rc;

	rc = phy_read(phydev, MII_KSZPHY_INTCS);

	return (rc < 0) ? rc : 0;
}

static int kszphy_set_interrupt(struct phy_device *phydev)
{
	int temp;
	temp = (PHY_INTERRUPT_ENABLED == phydev->interrupts) ?
		KSZPHY_INTCS_ALL : 0;
	return phy_write(phydev, MII_KSZPHY_INTCS, temp);
}

static int kszphy_config_intr(struct phy_device *phydev)
{
	int temp, rc;

	/* set the interrupt pin active low */
	temp = phy_read(phydev, MII_KSZPHY_CTRL);
	temp &= ~KSZPHY_CTRL_INT_ACTIVE_HIGH;
	phy_write(phydev, MII_KSZPHY_CTRL, temp);
	rc = kszphy_set_interrupt(phydev);
	return rc < 0 ? rc : 0;
}

static int ksz9021_config_intr(struct phy_device *phydev)
{
	int temp, rc;

	/* set the interrupt pin active low */
	temp = phy_read(phydev, MII_KSZPHY_CTRL);
	temp &= ~KSZ9021_CTRL_INT_ACTIVE_HIGH;
	phy_write(phydev, MII_KSZPHY_CTRL, temp);
	rc = kszphy_set_interrupt(phydev);
	return rc < 0 ? rc : 0;
}

static int ks8737_config_intr(struct phy_device *phydev)
{
	int temp, rc;

	/* set the interrupt pin active low */
	temp = phy_read(phydev, MII_KSZPHY_CTRL);
	temp &= ~KS8737_CTRL_INT_ACTIVE_HIGH;
	phy_write(phydev, MII_KSZPHY_CTRL, temp);
	rc = kszphy_set_interrupt(phydev);
	return rc < 0 ? rc : 0;
}

static int kszphy_config_init(struct phy_device *phydev)
{
	return 0;
}

static int ksz9021_config_init(struct phy_device *phydev)
{
	u16 val;

	/* clock and control data pad skew - register = 0x104 */
	val = (0x0 << MII_KSZ9021_EXT_RGMII_RXC_SHIFT)
		| (0x0 << MII_KSZ9021_EXT_RGMII_RX_DV_SHIFT)
		| (0x7 << MII_KSZ9021_EXT_RGMII_TXC_SHIFT)
		| (0x7 << MII_KSZ9021_EXT_RGMII_TX_EN_SHIFT);
	ksz9021_phy_extended_write(phydev, MII_KSZ9021_EXT_RGMII_CLOCK_SKEW, val);
	/* rx data pad skew - register = 0x105 */
	val = (0x0 << MII_KSZ90x1_EXT_RGMII_RX3_SHIFT)
		| (0x0 << MII_KSZ90x1_EXT_RGMII_RX2_SHIFT)
		| (0x0 << MII_KSZ90x1_EXT_RGMII_RX1_SHIFT)
		| (0x0 << MII_KSZ90x1_EXT_RGMII_RX0_SHIFT);
	ksz9021_phy_extended_write(phydev, MII_KSZ9021_EXT_RGMII_RX_DATA_SKEW, val);
	/* tx data pad skew - register = 0x106 */
	val = (0x7 << MII_KSZ90x1_EXT_RGMII_TX3_SHIFT)
		| (0x7 << MII_KSZ90x1_EXT_RGMII_TX2_SHIFT)
		| (0x7 << MII_KSZ90x1_EXT_RGMII_TX1_SHIFT)
		| (0x7 << MII_KSZ90x1_EXT_RGMII_TX0_SHIFT);
	ksz9021_phy_extended_write(phydev, MII_KSZ9021_EXT_RGMII_TX_DATA_SKEW, val);

	return 0;
}

static int ksz9031_config_init(struct phy_device *phydev)
{
	u16 val;

	/* control data pad skew - devaddr = 0x02, register = 0x04 */
	val = (0x0 << MII_KSZ9031_EXT_RGMII_RX_DV_SHIFT)
		| (0x7 << MII_KSZ9031_EXT_RGMII_TX_EN_SHIFT);
	ksz9031_phy_extended_write(phydev, 0x02,
					MII_KSZ9031_EXT_RGMII_CTRL_SIG_SKEW,
					MII_KSZ9031_MOD_DATA_NO_POST_INC, val);
	/* rx data pad skew - devaddr = 0x02, register = 0x05 */
	val = (0x0 << MII_KSZ90x1_EXT_RGMII_RX3_SHIFT)
		| (0x0 << MII_KSZ90x1_EXT_RGMII_RX2_SHIFT)
		| (0x0 << MII_KSZ90x1_EXT_RGMII_RX1_SHIFT)
		| (0x0 << MII_KSZ90x1_EXT_RGMII_RX0_SHIFT);
	ksz9031_phy_extended_write(phydev, 0x02,
					MII_KSZ9031_EXT_RGMII_RX_DATA_SKEW,
					MII_KSZ9031_MOD_DATA_NO_POST_INC, val);
	/* tx data pad skew - devaddr = 0x02, register = 0x06 */
	val = (0x7 << MII_KSZ90x1_EXT_RGMII_TX3_SHIFT)
		| (0x7 << MII_KSZ90x1_EXT_RGMII_TX2_SHIFT)
		| (0x7 << MII_KSZ90x1_EXT_RGMII_TX1_SHIFT)
		| (0x7 << MII_KSZ90x1_EXT_RGMII_TX0_SHIFT);
	ksz9031_phy_extended_write(phydev, 0x02,
					MII_KSZ9031_EXT_RGMII_TX_DATA_SKEW,
					MII_KSZ9031_MOD_DATA_NO_POST_INC, val);
	/* gtx and rx clock pad skew - devaddr = 0x02, register = 0x08 */
	val = (0x1F << MII_KSZ9031_EXT_RGMII_GTX_CLK_SHIFT)
		| (0x0E << MII_KSZ9031_EXT_RGMII_RX_CLK_SHIFT);
	ksz9031_phy_extended_write(phydev, 0x02,
					MII_KSZ9031_EXT_RGMII_CLOCK_SKEW,
					MII_KSZ9031_MOD_DATA_NO_POST_INC, val);

	return 0;
}

static int ks8051_config_init(struct phy_device *phydev)
{
	int regval;

	if (phydev->dev_flags & MICREL_PHY_50MHZ_CLK) {
		regval = phy_read(phydev, MII_KSZPHY_CTRL);
		regval |= KSZ8051_RMII_50MHZ_CLK;
		phy_write(phydev, MII_KSZPHY_CTRL, regval);
	}

	return 0;
}

static struct phy_driver ks8737_driver = {
	.phy_id		= PHY_ID_KS8737,
	.phy_id_mask	= 0x00fffff0,
	.name		= "Micrel KS8737",
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= kszphy_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= ks8737_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static struct phy_driver ks8041_driver = {
	.phy_id		= PHY_ID_KS8041,
	.phy_id_mask	= 0x00fffff0,
	.name		= "Micrel KS8041",
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= kszphy_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= kszphy_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static struct phy_driver ks8051_driver = {
	.phy_id		= PHY_ID_KS8051,
	.phy_id_mask	= 0x00fffff0,
	.name		= "Micrel KS8021, KS8031, or KS8051",
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= ks8051_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= kszphy_ack_interrupt,
	.driver		= { .owner = THIS_MODULE,},
};

static struct phy_driver ks8061_driver = {
	.phy_id		= PHY_ID_KS8061,
	.phy_id_mask	= 0x00fffff0,
	.name		= "Micrel KS8061",
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= ks8051_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= kszphy_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static struct phy_driver ks8081_driver = {
	.phy_id		= PHY_ID_KS8081,
	.name		= "Micrel KS8081 or KS8091",
	.phy_id_mask	= 0x00fffff0,
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= kszphy_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= kszphy_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static struct phy_driver ks8001_driver = {
	.phy_id		= PHY_ID_KS8001,
	.name		= "Micrel KS8001 or KS8721",
	.phy_id_mask	= 0x00fffff0,
	.features	= (PHY_BASIC_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= kszphy_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= kszphy_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

static struct phy_driver ksz9021_driver = {
	.phy_id		= PHY_ID_KSZ9021,
	.phy_id_mask	= 0x000fff10,
	.name		= "Micrel KSZ9021 Gigabit PHY",
	.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= ksz9021_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= ksz9021_config_intr,
	.driver		= { .owner = THIS_MODULE, },
};

static struct phy_driver ksz9031_driver = {
	.phy_id		= PHY_ID_KSZ9031,
	.phy_id_mask	= 0x00fffff0,
	.name		= "Micrel KSZ9031 Gigabit PHY",
	.features	= (PHY_GBIT_FEATURES | SUPPORTED_Pause),
	.flags		= PHY_HAS_MAGICANEG | PHY_HAS_INTERRUPT,
	.config_init	= ksz9031_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= kszphy_ack_interrupt,
	.config_intr	= ksz9021_config_intr,
	.driver		= { .owner = THIS_MODULE, },
};

static int __init ksphy_init(void)
{
	int ret;

	ret = phy_driver_register(&ks8001_driver);
	if (ret)
		goto err1;

	ret = phy_driver_register(&ksz9021_driver);
	if (ret)
		goto err2;

	ret = phy_driver_register(&ks8737_driver);
	if (ret)
		goto err3;
	ret = phy_driver_register(&ks8041_driver);
	if (ret)
		goto err4;
	ret = phy_driver_register(&ks8051_driver);
	if (ret)
		goto err5;

	ret = phy_driver_register(&ks8061_driver);
	if (ret)
		goto err6;

	ret = phy_driver_register(&ks8081_driver);
	if (ret)
		goto err7;

	ret = phy_driver_register(&ksz9031_driver);
	if (ret)
		goto err8;

	return 0;

err8:
	phy_driver_unregister(&ks8081_driver);
err7:
	phy_driver_unregister(&ks8061_driver);
err6:
	phy_driver_unregister(&ks8051_driver);
err5:
	phy_driver_unregister(&ks8041_driver);
err4:
	phy_driver_unregister(&ks8737_driver);
err3:
	phy_driver_unregister(&ksz9021_driver);
err2:
	phy_driver_unregister(&ks8001_driver);
err1:
	return ret;
}

static void __exit ksphy_exit(void)
{
	phy_driver_unregister(&ks8001_driver);
	phy_driver_unregister(&ks8737_driver);
	phy_driver_unregister(&ksz9021_driver);
	phy_driver_unregister(&ksz9031_driver);
	phy_driver_unregister(&ks8041_driver);
	phy_driver_unregister(&ks8051_driver);
	phy_driver_unregister(&ks8061_driver);
	phy_driver_unregister(&ks8081_driver);
}

module_init(ksphy_init);
module_exit(ksphy_exit);

MODULE_DESCRIPTION("Micrel PHY driver");
MODULE_AUTHOR("David J. Choi");
MODULE_LICENSE("GPL");

static struct mdio_device_id __maybe_unused micrel_tbl[] = {
	{ PHY_ID_KSZ9021, 0x00ffff10 },
	{ PHY_ID_KSZ9031, 0x00fffff0 },
	{ PHY_ID_KS8001, 0x00fffff0 },
	{ PHY_ID_KS8737, 0x00fffff0 },
	{ PHY_ID_KS8041, 0x00fffff0 },
	{ PHY_ID_KS8051, 0x00fffff0 },
	{ PHY_ID_KS8061, 0x00fffff0 },
	{ PHY_ID_KS8081, 0x00fffff0 },
	{ }
};

MODULE_DEVICE_TABLE(mdio, micrel_tbl);
