#pragma once

#include <zephyr/net/dhcpv4.h>
#include <zephyr/net/dhcpv4_server.h>
#include <cstddef>
#include <zephyr/kernel.h>
#include <zephyr/linker/sections.h>
#include <errno.h>
#include <stdio.h>

#include <zephyr/net/net_if.h>
#include <zephyr/net/net_core.h>
#include <zephyr/net/net_context.h>
#include <zephyr/net/net_mgmt.h>
#include <zephyr/net/ethernet.h>
#include <zephyr/net/ethernet_mgmt.h>

#include <api/IPAddress.h>

#define DHCP_OPTION_NTP (42)

class NetworkInterface {
private:
	uint8_t ntp_server[4];
	static struct net_mgmt_event_callback mgmt_cb;
	static struct net_dhcpv4_option_callback dhcp_cb;

	static void event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
							  struct net_if *iface);

	static void option_handler(struct net_dhcpv4_option_callback *cb, size_t length,
							   enum net_dhcpv4_msg_type msg_type, struct net_if *iface);

protected:
	struct net_if *netif = nullptr;
	int dhcp();
	void enable_dhcpv4_server(struct net_if *netif, char *_netmask = "255.255.255.0");

public:
	NetworkInterface() {
	}

	virtual ~NetworkInterface() {
	}

	void MACAddress(uint8_t *mac);
	IPAddress localIP();
	IPAddress subnetMask();
	IPAddress gatewayIP();
	IPAddress dnsServerIP();

	void config(const IPAddress ip, const IPAddress dns_server, const IPAddress gateway,
				const IPAddress subnet);
	void setMACAddress(const uint8_t *mac);
	void setLocalIP(const IPAddress ip);
	void setSubnetMask(const IPAddress subnet);
	void setGatewayIP(const IPAddress gateway);
	void setDnsServerIP(const IPAddress dns_server);

	int begin(bool blocking = true, uint32_t additional_event_mask = 0);

	bool disconnect();
};
