/*
 * Copyright 2025 AVSystem <avsystem@avsystem.com>
 * AVSystem Anjay LwM2M SDK
 * ALL RIGHTS RESERVED
 */
#ifndef MODEM_CONSTANTS_H
#define MODEM_CONSTANTS_H

// Some defaults per BG96 TCP/IP AT Commands Manual
#define MODEM_SOCKET_SEND_MAX 1460
#define MODEM_SOCKET_RECV_MAX 1500

// Assume that 256 bytes is enough for other other stuff like URC headers, etc.
#define MODEM_RX_BUF (MODEM_SOCKET_RECV_MAX + 256)
#define MODEM_TX_BUF (MODEM_SOCKET_SEND_MAX + 256)

#endif // MODEM_CONSTANTS_H
