/****************************************************************

Siano Mobile Silicon, Inc.
MDTV receiver kernel modules.
Copyright (C) 2006-2008, Uri Shkolnik

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 2 of the License, or
(at your option) any later version.

 This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.

****************************************************************/

#include <linux/module.h>
#include <linux/init.h>
#include <linux/netdevice.h>	/* struct device, and other headers */
#include <linux/etherdevice.h>	/* eth_type_trans */
#include <linux/ip.h>		/* struct iphdr */
#include <linux/ipv6.h>		/* struct ipv6hdr */
#include <linux/in.h>
#include <linux/slab.h>

#include "smscoreapi.h"


#define IPV4VERSION			0x40
#define IPV6VERSION			0x60
#define GETIPVERSION(_x_)	((_x_) & 0xf0)

struct smsnet_client_t {
	struct list_head entry;

	struct smscore_device_t *coredev;
	struct smscore_client_t *smsclient;

	int packet_length, splitpacket_length;
	int header_length, splitheader_length;
	u8 splitpacket[ETH_DATA_LEN];
};

struct list_head g_smsnet_clients;
struct mutex g_smsnet_clientslock;

struct net_device *g_smsnet_device;
struct net_device_stats g_smsnet_stats;

int g_smsnet_inuse;

void smsnet_send_packet(u8 *buffer, int length)
{
	u8 *eth;
	struct sk_buff *skb = dev_alloc_skb(length + ETH_HLEN + NET_IP_ALIGN);

	if (!skb) {
		g_smsnet_stats.rx_dropped++;
		return;
	}

	skb_reserve(skb, NET_IP_ALIGN);

	eth = (u8 *) skb_put(skb, length + ETH_HLEN);
	memcpy(eth + ETH_HLEN, buffer, length);

	eth[6] = 0;
	eth[7] = 1;
	eth[8] = 1;
	eth[9] = 3;
	eth[10] = 4;
	eth[11] = 5;

	if (GETIPVERSION(*buffer) == IPV4VERSION) {
		eth[0] = 1;
		eth[1] = 0;
		eth[2] = 0x5e;
		eth[3] = buffer[17] & 0x7f;
		eth[4] = buffer[18];
		eth[5] = buffer[19];

		eth[12] = 0x08;
		eth[13] = 0x00;
	} else {
		/* ip6 mcast address */
		eth[0] = 0x33;
		eth[1] = 0x33;
		eth[2] = buffer[36];
		eth[3] = buffer[37];
		eth[4] = buffer[38];
		eth[5] = buffer[39];

		eth[12] = 0x86;
		eth[13] = 0xdd;
	}

	skb->dev = g_smsnet_device;
	skb->protocol = eth_type_trans(skb, g_smsnet_device);
	skb->ip_summed = CHECKSUM_COMPLETE;

	g_smsnet_stats.rx_packets++;
	g_smsnet_stats.rx_bytes += skb->len;

	netif_rx(skb);
}

int check_header(struct smsnet_client_t *client, u8 *buffer)
{
	struct iphdr *ip4_hdr;
	struct ipv6hdr *ip6_hdr;
	struct udphdr *udp_hdr;
	u16 csum;

	/* check if packet header is valid and it is a UDP */
	if (GETIPVERSION(*buffer) == IPV4VERSION) {
		ip4_hdr = (struct iphdr *)buffer;
		csum = ip4_hdr->check;

		ip4_hdr->check = 0;

		/* check header checksum for IPv4 packets */
		if (ip4_hdr->protocol != IPPROTO_UDP || csum !=
		    ip_fast_csum(buffer, ip4_hdr->ihl)) {
			ip4_hdr->check = csum;
			return 0;
		}

		ip4_hdr->check = csum;
		client->packet_length = ntohs(ip4_hdr->tot_len);
	} else {
		ip6_hdr = (struct ipv6hdr *)buffer;
		udp_hdr = (struct udphdr *)(ip6_hdr + 1);

		if ((ip6_hdr->nexthdr != IPPROTO_UDP) ||
		    (ip6_hdr->payload_len != udp_hdr->len))
			return 0;

		client->packet_length = ntohs(ip6_hdr->payload_len) +
		    sizeof(struct ipv6hdr);
	}

	/* check for abnormal packet length */
	if (client->packet_length > ETH_DATA_LEN)
		return 0;

	return 1;
}

int smsnet_onresponse(void *context, struct smscore_buffer_t *cb)
{
	struct smsnet_client_t *client = (struct smsnet_client_t *)context;
	int length, rest;
	u8 ip_ver, *buffer;

	buffer = ((u8 *) cb->p) + cb->offset + sizeof(struct SmsMsgHdr_S);
	length = cb->size - sizeof(struct SmsMsgHdr_S);

	if (client->splitheader_length) {
		/* how much data is missing ? */
		rest = client->header_length - client->splitheader_length;

		/* do we have enough in this buffer ? */
		rest = min(rest, length);

		memcpy(&client->splitpacket[client->splitheader_length],
		       buffer, rest);

		client->splitheader_length += rest;

		if (client->splitheader_length != client->header_length)
			goto exit;

		if (check_header(client, client->splitpacket)) {
			buffer += rest;
			length -= rest;

			client->splitpacket_length = client->header_length;
		}

		client->splitheader_length = 0;
	}

	if (client->splitpacket_length) {
		/* how much data is missing ? */
		rest = client->packet_length - client->splitpacket_length;

		/* do we have enough in this buffer ? */
		rest = min(rest, length);

		memcpy(&client->splitpacket[client->splitpacket_length],
		       buffer, rest);

		client->splitpacket_length += rest;

		if (client->splitpacket_length != client->packet_length)
			goto exit;

		client->splitpacket_length = 0;

		smsnet_send_packet(client->splitpacket, client->packet_length);

		buffer += rest;
		length -= rest;
	}

	while (length > 0) {
		ip_ver = GETIPVERSION(*buffer);
		while (length && (ip_ver != IPV4VERSION) &&
		       (ip_ver != IPV6VERSION)) {
			buffer++;
			length--;
			ip_ver = GETIPVERSION(*buffer);
		}

		/* No more data in section */
		if (!length)
			break;

		/* Set the header length at start of packet according
		   to the version no problem with the IP header cast, since
		   we have at least 1 byte (we use only the first byte) */
		client->header_length =
		    (ip_ver == IPV4VERSION) ?
		    (((struct iphdr *)buffer)->ihl * 4) :
		    (sizeof(struct ipv6hdr) + sizeof(struct udphdr));

		/*Check that Header length is at least 20 (min IPv4 length) */
		if (client->header_length < 20) {
			length--;
			buffer++;
			continue;
		}

		/* check split header case */
		if (client->header_length > length) {
			memcpy(client->splitpacket, buffer, length);
			client->splitheader_length = length;
			break;
		}

		if (check_header(client, buffer)) {
			/* check split packet case */
			if (client->packet_length > length) {
				memcpy(client->splitpacket, buffer, length);
				client->splitpacket_length = length;
				break;
			}
		} else {
			length--;
			buffer++;
			continue;
		}

		smsnet_send_packet(buffer, client->packet_length);

		buffer += client->packet_length;
		length -= client->packet_length;
	}

exit:
	smscore_putbuffer(client->coredev, cb);

	return 0;
}

void smsnet_unregister_client(struct smsnet_client_t *client)
{
	/* must be called under clientslock */

	list_del(&client->entry);

	smscore_unregister_client(client->smsclient);
	kfree(client);
}

void smsnet_onremove(void *context)
{
	kmutex_lock(&g_smsnet_clientslock);

	smsnet_unregister_client((struct smsnet_client_t *)context);

	kmutex_unlock(&g_smsnet_clientslock);
}

int smsnet_hotplug(void *coredev, struct device *device,
		   int arrival)
{
	struct smsclient_params_t params;
	struct smsnet_client_t *client;
	int rc;

	/* device removal handled by onremove callback */
	if (!arrival)
		return 0;

	client = kzalloc(sizeof(struct smsnet_client_t), GFP_KERNEL);
	if (!client) {
		sms_err("kmalloc() failed");
		return -ENOMEM;
	}

	params.initial_id = 1;
	params.data_type = MSG_SMS_DATA_MSG;
	params.onresponse_handler = smsnet_onresponse;
	params.onremove_handler = smsnet_onremove;
	params.context = client;

	rc = smscore_register_client(coredev, &params, &client->smsclient);
	if (rc < 0) {
		sms_err("smscore_register_client() failed %d", rc);
		kfree(client);
		return rc;
	}

	client->coredev = coredev;
	kmutex_lock(&g_smsnet_clientslock);
	list_add(&client->entry, &g_smsnet_clients);
	kmutex_unlock(&g_smsnet_clientslock);
	sms_info("success");
	return 0;
}

static int smsnet_open(struct net_device *dev)
{
	g_smsnet_inuse++;

	netif_start_queue(dev);
	sms_info("smsnet in use %d", g_smsnet_inuse);
	return 0;
}

static int smsnet_stop(struct net_device *dev)
{
	netif_stop_queue(dev);
	g_smsnet_inuse--;
	sms_info("smsnet in use %d", g_smsnet_inuse);
	return 0;
}

static int smsnet_hard_start_xmit(struct sk_buff *skb, struct net_device *dev)
{
	sms_info("enter");
	dev_kfree_skb(skb);
	return 0;
}

static struct net_device_stats *smsnet_get_stats(struct net_device *dev)
{
	return &g_smsnet_stats;
}

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29) 
static const struct net_device_ops sms_netdev_ops = { 
    .ndo_open               = smsnet_open, 
    .ndo_stop               = smsnet_stop, 
    .ndo_start_xmit         = smsnet_hard_start_xmit, 
    .ndo_tx_timeout         = NULL,
    .ndo_do_ioctl           = NULL,
    .ndo_get_stats          = smsnet_get_stats,
    .ndo_set_mac_address    = NULL,
    .ndo_change_mtu         = NULL,
}; 
#endif

static void smsnet_setup_device(struct net_device *dev)
{
	ether_setup(dev);

#if LINUX_VERSION_CODE >= KERNEL_VERSION(2,6,29)	
    dev->netdev_ops = &sms_netdev_ops;
#else
    dev->open = smsnet_open;
    dev->stop = smsnet_stop;
    dev->hard_start_xmit = smsnet_hard_start_xmit;
    dev->get_stats = smsnet_get_stats;
    dev->set_multicast_list = smsnet_set_multicast_list;
#endif

#if LINUX_VERSION_CODE < KERNEL_VERSION(2,6,34)
	dev->mc_count = 0;
#endif

	memcpy(dev->dev_addr, "\0SIANO", ETH_ALEN);

	dev->flags |= IFF_NOARP | IFF_MULTICAST;
	dev->features |= NETIF_F_IP_CSUM;

}

int smsnet_register(void)
{
	int rc;

	INIT_LIST_HEAD(&g_smsnet_clients);
	kmutex_init(&g_smsnet_clientslock);

	memset(&g_smsnet_stats, 0, sizeof(g_smsnet_stats));

	g_smsnet_device = alloc_netdev(0, "sms", smsnet_setup_device);
	if (!g_smsnet_device) {
		sms_err("alloc_netdev() failed");
		return -ENOMEM;
	}

	rc = register_netdev(g_smsnet_device);
	if (rc < 0) {
		sms_err("register_netdev() failed %d\n", rc);
		free_netdev(g_smsnet_device);
		return rc;
	}

	rc = smscore_register_hotplug(smsnet_hotplug);
	sms_info("exit - rc %d", rc);

	return rc;
}

void smsnet_unregister(void)
{
	if (g_smsnet_device) {
		unregister_netdev(g_smsnet_device);
		free_netdev(g_smsnet_device);

		g_smsnet_device = NULL;
	}

	smscore_unregister_hotplug(smsnet_hotplug);

	kmutex_lock(&g_smsnet_clientslock);

	while (!list_empty(&g_smsnet_clients))
		smsnet_unregister_client((struct smsnet_client_t *)
					 g_smsnet_clients.next);

	kmutex_unlock(&g_smsnet_clientslock);

	sms_info("exit");
}

