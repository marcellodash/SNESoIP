/* firmware.c -*-c-*-
 * SNESoIP firmware.
 * Copyright (c) 2014 Michael Fitzmayer.  All rights reserved.
 *
 * This program has has been released under the terms of a BSD-like
 * license.  See the file LICENSE for details. */


#include "firmware.h"


int main(void) {
	uint8_t  buffer[BUFFER_SIZE + 1];


	// Initialise UART.
	DEBUG_INIT();
	uart_puts("\r\nWelcome to the SNESoIP debugging interface.\r\n\n");


	// Initialise basic I/O.
	uart_puts("Initialise basic I/O.\r\n");
	initLed();
	ledOnRed();
	INIT_IO();


	// Initialise network interface.
	uart_puts("Initialise network interface.\n");
	initNetwork();


	// Get the initial IP via DHCP and configure network.
	uart_puts("\rGet the initial IP via DHCP and configure network: ");
	printArray(setIPviaDHCP(buffer), 4, 10, '.');
	uart_puts("\r\n");


	// Resolve MAC address from server or gateway.
	uart_puts("Resolve MAC address from server or gateway: ");
	printArray(resolveMAC(buffer), 6, 16, ':');
	uart_puts("\r\n");


	// Perform DNS lookup of server hostname.
	uart_puts("Perform DNS lookup of server hostname: ");
	printArray(dnsLookup(buffer, "snesoip.de"), 4, 10, '.');
	uart_puts("\r\n");


	// Connected.
	uart_puts("Connected.\r\n");
	ledOnGreen();


	while (1) {
		uint16_t received;
		snesIO   port0 = 0xffff;
		received = enc28j60PacketReceive(BUFFER_SIZE, buffer);


		// Do something while no packet in queue.
		if (received == 0) {
			port0 = recvInput();

			sendOutput(port0, port0);
			continue;
		}

		// Check if IP packets (ICMP or UDP) are for us.
		if (eth_type_is_ip_and_my_ip(buffer, received) == 0)
			continue;


		// Answer ping with pong.
		if (
			buffer[IP_PROTO_P]  == IP_PROTO_ICMP_V &&
			buffer[ICMP_TYPE_P] == ICMP_TYPE_ECHOREQUEST_V) {

			uart_puts("Pong.\r\n");
			make_echo_reply_from_request(buffer, received);
			continue;
		}

	}


	return (0);
}