/*
 * Tty port functions for RFCOMM v1.11
 *
 * Copy from drivers/tty/tty_port.c of v3.18.25
 */

#ifdef COMPAT_TTY_PORT

/* for tty_driver->ports */
static struct tty_port *rfcomm_ports[RFCOMM_TTY_PORTS];

void tty_port_link_device(struct tty_port *port,
		struct tty_driver *driver, unsigned index)
{
	if (WARN_ON(index >= driver->num))
		return;
	rfcomm_ports[index] = port;
}

struct device *tty_port_register_device(struct tty_port *port,
		struct tty_driver *driver, unsigned index,
		struct device *device)
{
	tty_port_link_device(port, driver, index);
	return tty_register_device(driver, index, device);
}

void tty_port_tty_hangup(struct tty_port *port, bool check_clocal)
{
	struct tty_struct *tty = tty_port_tty_get(port);

	if (tty && (!check_clocal || !C_CLOCAL(tty)))
		tty_hangup(tty);
	tty_kref_put(tty);
}

void tty_port_tty_wakeup(struct tty_port *port)
{
	struct tty_struct *tty = tty_port_tty_get(port);

	if (tty) {
		tty_wakeup(tty);
		tty_kref_put(tty);
	}
}

int tty_port_install(struct tty_port *port, struct tty_driver *driver,
		struct tty_struct *tty)
{
	tty->port = port;
	return tty_standard_install(driver, tty);
}

#endif /* COMPAT_TTY_PORT */
