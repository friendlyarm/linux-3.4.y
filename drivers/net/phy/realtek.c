/*
 * drivers/net/phy/realtek.c
 *
 * Driver for Realtek PHYs
 *
 * Author: Johnson Leung <r58129@freescale.com>
 *
 * Copyright (c) 2004 Freescale Semiconductor, Inc.
 *
 * This program is free software; you can redistribute  it and/or modify it
 * under  the terms of  the GNU General  Public License as published by the
 * Free Software Foundation;  either version 2 of the  License, or (at your
 * option) any later version.
 *
 */
#include <linux/phy.h>
#include <linux/module.h>

#define RTL821x_PHYSR		0x11
#define RTL821x_PHYSR_DUPLEX	0x2000
#define RTL821x_PHYSR_SPEED	0xc000
/* page 0 register 30 - interrupt indicators and SNR display register */
#define RTL8201F_INSR		0x1e
/* page 0 register 31 - page select register */
#define RTL8201F_PGSR		0x1f
/* page 7 register 19 - interrupt, WOL enable, and LEDs function register */
#define RTL8201F_INER		0x13
#define RTL8201F_INER_MASK	0x3800

#define RTL821x_INER		0x12
#define RTL821x_INER_INIT	0x6400
#define RTL821x_INSR		0x13

#define RTL8211E_INER_LINK_STATUS	0x400

#define RTL8211E_PGSR		0x1f

#define CTRL1000_PREFER_MASTER		(1 << 10)
#define CTRL1000_CONFIG_MASTER		(1 << 11)
#define CTRL1000_MANUAL_CONFIG		(1 << 12)

MODULE_DESCRIPTION("Realtek PHY driver");
MODULE_AUTHOR("Johnson Leung");
MODULE_LICENSE("GPL");

static int rtl8201f_ack_interrupt(struct phy_device *phydev)
{
#if 0
	int err;

	err = phy_read(phydev, RTL8201F_INSR);

    return (err < 0) ? err : 0; 
#endif 
    return 0;
}

static int rtl821x_ack_interrupt(struct phy_device *phydev)
{
	int err;

	err = phy_read(phydev, RTL821x_INSR);

	if (err & 0x300) {								/* False Carrier or Symbol Error */
		phy_write(phydev, RTL8211E_PGSR, 0x0000);	/* phy reset */
		phy_write(phydev, 0, 0x8000);
	}

	return (err < 0) ? err : 0;
}

static int rtl8201f_config_intr(struct phy_device *phydev)
{
#if 0
	int err;

	phy_write(phydev, RTL8201F_PGSR, 0x0007);	/* select page 7 */

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL8201F_INER, RTL8201F_INER_MASK |
						phy_read(phydev, RTL8201F_INER));
	else
		err = phy_write(phydev, RTL8201F_INER, ~RTL8201F_INER_MASK &
						phy_read(phydev, RTL8201F_INER));

	phy_write(phydev, RTL8201F_PGSR, 0x0000);	/* back to page 0 */

	return err;
#endif
    return 0;
}

static int rtl8211b_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL821x_INER_INIT);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211e_config_intr(struct phy_device *phydev)
{
	int err;

	if (phydev->interrupts == PHY_INTERRUPT_ENABLED)
		err = phy_write(phydev, RTL821x_INER,
				RTL8211E_INER_LINK_STATUS | 0x300);
	else
		err = phy_write(phydev, RTL821x_INER, 0);

	return err;
}

static int rtl8211e_config_init(struct phy_device *phydev)
{
	return 0;
}

/* RTL8201F */
static struct phy_driver rtl8201f_driver = {
	.phy_id		= 0x001cc816,
	.name		= "RTL8201F 10/100Mbps Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_BASIC_FEATURES,
	.flags		= 0,
	.config_aneg	= &genphy_config_aneg,
	.read_status	= &genphy_read_status,
	.ack_interrupt	= &rtl8201f_ack_interrupt,
	.config_intr	= &rtl8201f_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

/* RTL8211B */
static struct phy_driver rtl8211b_driver = {
	.phy_id		= 0x001cc912,
	.name		= "RTL8211B Gigabit Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_GBIT_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= rtl821x_ack_interrupt,
	.config_intr	= rtl8211b_config_intr,
	.driver		= { .owner = THIS_MODULE,},
};

/* RTL8211E */
static struct phy_driver rtl8211e_driver = {
	.phy_id		= 0x001cc915,
	.name		= "RTL8211E Gigabit Ethernet",
	.phy_id_mask	= 0x001fffff,
	.features	= PHY_GBIT_FEATURES,
	.flags		= PHY_HAS_INTERRUPT,
	.config_init	= rtl8211e_config_init,
	.config_aneg	= genphy_config_aneg,
	.read_status	= genphy_read_status,
	.ack_interrupt	= rtl821x_ack_interrupt,
	.config_intr	= rtl8211e_config_intr,
#if !defined(CFG_ETHER_LOOPBACK_MODE) || CFG_ETHER_LOOPBACK_MODE == 0
	.suspend	= genphy_suspend,
	.resume		= genphy_resume,
#endif
	.driver		= { .owner = THIS_MODULE,},
};

static int __init realtek_init(void)
{
	if (phy_driver_register(&rtl8201f_driver) < 0)
		return -ENODEV;
	if (phy_driver_register(&rtl8211b_driver) < 0)
		return -ENODEV;
	return phy_driver_register(&rtl8211e_driver);
}

static void __exit realtek_exit(void)
{
	phy_driver_unregister(&rtl8201f_driver);
	phy_driver_unregister(&rtl8211b_driver);
	phy_driver_unregister(&rtl8211e_driver);
}

module_init(realtek_init);
module_exit(realtek_exit);

static struct mdio_device_id __maybe_unused realtek_tbl[] = {
	{ 0x001cc816, 0x001fffff },
	{ 0x001cc912, 0x001fffff },
	{ 0x001cc915, 0x001fffff },
	{ }
};

MODULE_DEVICE_TABLE(mdio, realtek_tbl);
