#include "SocketHelpers.h"

#include <zephyr/logging/log.h>
LOG_MODULE_DECLARE(sketch, LOG_LEVEL_NONE);

struct net_mgmt_event_callback NetworkInterface::mgmt_cb;
struct net_dhcpv4_option_callback NetworkInterface::dhcp_cb;

void NetworkInterface::event_handler(struct net_mgmt_event_callback *cb, uint64_t mgmt_event,
									 struct net_if *iface) {
	int i = 0;

	if (mgmt_event != NET_EVENT_IPV4_ADDR_ADD) {
		return;
	}

	for (i = 0; i < NET_IF_MAX_IPV4_ADDR; i++) {
		char buf[NET_IPV4_ADDR_LEN];

		if (iface->config.ip.ipv4->unicast[i].ipv4.addr_type != NET_ADDR_DHCP) {
			continue;
		}

		LOG_INF("   Address[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[i].ipv4.address.in_addr, buf,
							  sizeof(buf)));
		LOG_INF(
			"    Subnet[%d]: %s", net_if_get_by_iface(iface),
			net_addr_ntop(AF_INET, &iface->config.ip.ipv4->unicast[i].netmask, buf, sizeof(buf)));
		LOG_INF("    Router[%d]: %s", net_if_get_by_iface(iface),
				net_addr_ntop(AF_INET, &iface->config.ip.ipv4->gw, buf, sizeof(buf)));
		LOG_INF("Lease time[%d]: %u seconds", net_if_get_by_iface(iface),
				iface->config.dhcpv4.lease_time);
	}
}

void NetworkInterface::option_handler(struct net_dhcpv4_option_callback *cb, size_t length,
									  enum net_dhcpv4_msg_type msg_type, struct net_if *iface) {
	char buf[NET_IPV4_ADDR_LEN];

	LOG_INF("DHCP Option %d: %s", cb->option, net_addr_ntop(AF_INET, cb->data, buf, sizeof(buf)));
}

int NetworkInterface::dhcp() {
	net_mgmt_init_event_callback(&mgmt_cb, event_handler,
								 NET_EVENT_IPV4_ADDR_ADD | NET_EVENT_IF_UP | NET_EVENT_IF_DOWN);
	net_mgmt_add_event_callback(&mgmt_cb);

	net_dhcpv4_init_option_callback(&dhcp_cb, option_handler, DHCP_OPTION_NTP, ntp_server,
									sizeof(ntp_server));

	net_dhcpv4_add_option_callback(&dhcp_cb);

	net_dhcpv4_start(netif);

	LOG_INF("DHCPv4 started...\n");

	return 0;
}

void NetworkInterface::enable_dhcpv4_server(struct net_if *netif, char *_netmask) {
	static struct in_addr addr;
	static struct in_addr netmaskAddr;

	if (net_addr_pton(AF_INET, String(localIP()).c_str(), &addr)) {
		LOG_ERR("Invalid address: %s", String(localIP()).c_str());
		return;
	}

	if (net_addr_pton(AF_INET, _netmask, &netmaskAddr)) {
		LOG_ERR("Invalid netmask: %s", _netmask);
		return;
	}

	net_if_ipv4_set_gw(netif, &addr);

	if (net_if_ipv4_addr_add(netif, &addr, NET_ADDR_MANUAL, 0) == NULL) {
		LOG_ERR("unable to set IP address for AP interface");
	}

	if (!net_if_ipv4_set_netmask_by_addr(netif, &addr, &netmaskAddr)) {
		LOG_ERR("Unable to set netmask for AP interface: %s", _netmask);
	}

	addr.s4_addr[3] += 10; /* Starting IPv4 address for DHCPv4 address pool. */

	if (net_dhcpv4_server_start(netif, &addr) != 0) {
		LOG_ERR("DHCP server is not started for desired IP");
		return;
	}

	LOG_INF("DHCPv4 server started...\n");
}

void NetworkInterface::MACAddress(uint8_t *mac) {
	if (netif == NULL || netif->if_dev == NULL || mac == NULL) {
		return;
	}
	memcpy(mac, netif->if_dev->link_addr.addr, netif->if_dev->link_addr.len);
}

IPAddress NetworkInterface::localIP() {
	return IPAddress(netif->config.ip.ipv4->unicast[0].ipv4.address.in_addr.s_addr);
}

IPAddress NetworkInterface::subnetMask() {
	return IPAddress(netif->config.ip.ipv4->unicast[0].netmask.s_addr);
}

IPAddress NetworkInterface::gatewayIP() {
	return IPAddress(netif->config.ip.ipv4->gw.s_addr);
}

IPAddress NetworkInterface::dnsServerIP() {
	return arduino::INADDR_NONE;
}

void NetworkInterface::setMACAddress(const uint8_t *mac) {
	struct ethernet_req_params params = {0};

	if (mac == nullptr) {
		LOG_ERR("MAC address pointer is null");
		return;
	}
	memcpy(&params.mac_address, mac, sizeof(struct net_eth_addr));
	if (!net_eth_is_addr_valid(&params.mac_address)) {
		LOG_ERR("Invalid MAC address");
		return;
	}

	net_if_down(netif); // Ensure the interface is down before changing the MAC address

	int ret = net_mgmt(NET_REQUEST_ETHERNET_SET_MAC_ADDRESS, netif, &params, sizeof(params));
	if (ret != 0) {
		LOG_ERR("Failed to set MAC address via net_mgmt, ret=%d", ret);
	} else {
		LOG_INF("MAC address set successfully via net_mgmt");
	}

	net_if_up(netif); // Bring the interface back up after changing the MAC address
}

int NetworkInterface::begin(bool blocking, uint32_t additional_event_mask) {
	dhcp();
	int ret = net_mgmt_event_wait_on_iface(netif, NET_EVENT_IPV4_ADDR_ADD | additional_event_mask,
										   NULL, NULL, NULL, blocking ? K_FOREVER : K_SECONDS(1));
	return (ret == 0) ? 1 : 0;
}

bool NetworkInterface::disconnect() {
	return (net_if_down(netif) == 0);
}

void NetworkInterface::config(const IPAddress ip, const IPAddress dns_server,
							  const IPAddress gateway, const IPAddress subnet) {
	setLocalIP(ip);
	setDnsServerIP(dns_server);
	setGatewayIP(gateway);
	setSubnetMask(subnet);
	return;
}

void NetworkInterface::setLocalIP(const IPAddress ip) {
	struct in_addr addr;
	addr.s_addr = ip;

	if (!net_if_ipv4_addr_add(netif, &addr, NET_ADDR_MANUAL, 0)) {
		LOG_ERR("Failed to set local IP address");
		return;
	}
	LOG_INF("Local IP address set: %s", ip.toString().c_str());
	return;
}

void NetworkInterface::setSubnetMask(const IPAddress subnet) {
	struct in_addr netmask_addr;
	netmask_addr.s_addr = subnet;
	net_if_ipv4_set_netmask(netif, &netmask_addr);
	LOG_INF("Subnet mask set: %s", subnet.toString().c_str());
	return;
}

void NetworkInterface::setGatewayIP(const IPAddress gateway) {
	struct in_addr gw_addr;
	gw_addr.s_addr = gateway;
	net_if_ipv4_set_gw(netif, &gw_addr);
	LOG_INF("Gateway IP set: %s", gateway.toString().c_str());
	return;
}

void NetworkInterface::setDnsServerIP(const IPAddress dns_server) {
	return; // DNS server dynamic configuration is not supported
}
