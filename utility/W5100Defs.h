////////////////////////////////////////////////////////////////////////////////
//
//  W5100Defs.h - Definition of W5100 driver
//  ECA 05Oct2013
//
//  ----------------------
//
//  IMPORTANT: Refer to WIZnet W5100 datasheet for a complete description
//  of the chip and its operation.
//
//  The W5100 is a full-featured, single-chip internet enabled 10/100 Ethernet
//  controller designed for embedded applications. It has been designed to
//  facilitate easy implementation of internet connectivity without OS.
//
//  The W5100 programming model is based on simple socket programming by means
//  of a set of registers and 16KB memory for TX and RX buffers.
//  The W5100 supports 4 independent sockets simultaneously.
//
//  The W5100 memory map is composed of Common Registers, Socket Registers,
//  TX and RX memory, as described below:
//
//  0x0000  +-------------------------+
//          |    Common Registers     |   48 bytes
//  0x0030  +-------------------------+
//          |        Reserved         |
//  0x0400  +-------------------------+
//          |    Socket Registers     |   4 x 256 bytes
//  0x0800  +-------------------------+
//          |        Reserved         |
//  0x4000  +-------------------------+
//          |                         |
//          |       TX memory         |   8192 bytes
//          |                         |
//  0x6000  +-------------------------+
//          |                         |
//          |       RX memory         |   8192 bytes
//          |                         |
//  0x8000  +-------------------------+
//
//  ----------------------
//
// This file is free software; you can redistribute it and/or modify
// it under the terms of either the GNU General Public License version 2
// or the GNU Lesser General Public License version 2.1, both as
// published by the Free Software Foundation.
//
////////////////////////////////////////////////////////////////////////////////

#ifndef W5100DEFS_H
#define W5100DEFS_H

#define W5100_DBG_PIN0                22  // Customize this according to your configuration
#define W5100_DBG_PIN1                23  // Customize this according to your configuration

////////////////////////////////////////////////////////////////////////////////
// Memory Map - Sections
#define W5100_MEM_BASE                0x0000

#define W5100_MEM_COMMON_REGS_BASE    (W5100_MEM_BASE + 0x0000)
#define W5100_MEM_COMMON_REGS_SIZE    0x0030

#define W5100_MEM_SOCKET_REGS_BASE    (W5100_MEM_BASE + 0x0400)
#define W5100_MEM_SOCKET_REGS_SIZE    0x0100
#define W5100_MEM_S0_REGS_BASE        (W5100_MEM_SOCKET_REGS_BASE + 0x0000)
#define W5100_MEM_S1_REGS_BASE        (W5100_MEM_SOCKET_REGS_BASE + 0x0100)
#define W5100_MEM_S2_REGS_BASE        (W5100_MEM_SOCKET_REGS_BASE + 0x0200)
#define W5100_MEM_S3_REGS_BASE        (W5100_MEM_SOCKET_REGS_BASE + 0x0300)

#define W5100_MEM_TX_BASE             (W5100_MEM_BASE + 0x4000)
#define W5100_MEM_TX_SIZE             0x2000

#define W5100_MEM_RX_BASE             (W5100_MEM_BASE + 0x6000)
#define W5100_MEM_RX_SIZE             0x2000

////////////////////////////////////////////////////////////////////////////////
// Memory Map - Common registers

// MR (Mode Register) [R/W] [0x0000][0x00]
// This register is used for S/W reset, ping block mode, PPPoE mode and indirect buf I/F
#define W5100_MR                    0x0000

#define W5100_RST                   0x80    // Software reset (1:reset, automatically cleared after reset)
#define W5100_PB                    0x10    // Ping Block Mode (1:enable; 0:disable)
#define W5100_PPPoE                 0x08    // PPPoE Mode (1:enable; 0:disable)
#define W5100_AI                    0x02    // Address Auto-Increment in Indirect Bus I/F (1:enable; 0:disable)
#define W5100_IND                   0x01    // Indirect Bus I/F Mode (1:enable; 0:disable)

// GAR (Gateway IP Address Register) [R/W] [0x0001-0x0004][0x00]
// This register sets up the default gateway address
// Example: "192.168.0.1"
#define W5100_GAR0                  0x0001  // 192 (0xC0)
#define W5100_GAR1                  0x0002  // 168 (0xA8)
#define W5100_GAR2                  0x0003  // 0   (0x00)
#define W5100_GAR3                  0x0004  // 1   (0x01)

// SUBR (Subnet Mask Register) [R/W] [0x0005-0x0008][0x00]
// This register sets up the IP subnet mask
// Example: "255.255.255.0"
#define W5100_SUBR0                 0x0005  // 255 (0xFF)
#define W5100_SUBR1                 0x0006  // 255 (0xFF)
#define W5100_SUBR2                 0x0007  // 255 (0xFF)
#define W5100_SUBR3                 0x0008  // 0   (0x00)

// SHAR (Source Hardware Address Register) [R/W] [0x0009-0x000E][0x00]
// This register sets up the source hardware address
// Example: "00.08.DC.01.02.03"
#define W5100_SHAR0                 0x0009  // 0x00
#define W5100_SHAR1                 0x000A  // 0x08
#define W5100_SHAR2                 0x000B  // 0xDC
#define W5100_SHAR3                 0x000C  // 0x01
#define W5100_SHAR4                 0x000D  // 0x02
#define W5100_SHAR5                 0x000E  // 0x03

// SIPR (Source IP Address Register) [R/W] [0x000F-0x0012][0x00]
// This register sets up the source IP address
// Example : "192.168.0.3"
#define W5100_SIPR0                 0x000F  // 192 (0xC0)
#define W5100_SIPR1                 0x0010  // 168 (0xA8)
#define W5100_SIPR2                 0x0011  // 0   (0x00)
#define W5100_SIPR3                 0x0012  // 3   (0x03)

// IR (Interrupt Register) [R] [0x0015][0x00]
// This register is accessed by the host processor to know the cause of an interrupt
#define W5100_IR                    0x0015

#define W5100_CONFLICT              0x80    // IP Conflict (set to 1 when triggered; write 1 to clear)
#define W5100_UNREACH               0x40    // Destination unreachable (set to 1 when triggered; write 1 to clear)
#define W5100_PPPoE                 0x20    // PPPoE Connection Close (set to 1 when triggered; write 1 to clear)
#define W5100_S3_INT                0x08    // Occurrence of Socket 3 Socket Interrupt (set to 1 when triggered; write 1 to clear)
#define W5100_S2_INT                0x04    // Occurrence of Socket 2 Socket Interrupt (set to 1 when triggered; write 1 to clear)
#define W5100_S1_INT                0x02    // Occurrence of Socket 1 Socket Interrupt (set to 1 when triggered; write 1 to clear)
#define W5100_S0_INT                0x01    // Occurrence of Socket 0 Socket Interrupt (set to 1 when triggered; write 1 to clear)

// IMR (Interrupt Mask Register) [R/W] [0x0016][0x00]
// The Interrupt Mask Register is used to mask interrupts. 1->enable; 0->disable
// Bit mask names: use the above
// 1: active; 0:masked
#define W5100_IMR                   0x0016

// RTR (Retry Time-value Register) [R/W] [0x0017-0x0018][0x07D0]
// This register sets the period of timeout. Value 1 means 100us.
// The initial value is 2000 (0x07D0) corresponding to 200ms
#define W5100_RTR0                  0x0017  // (0x07)
#define W5100_RTR1                  0x0018  // (0xD0)

// RCR (Retry Count Register) [R/W] [0x0019][0x08]
// This register sets the number of re-transmission
#define W5100_RCR                   0x0019

// RMSR (RX Memory Size Register) [R/W] [0x001A][0x55]
// This register assigns total 8K RX memory to each socket
#define W5100_RMSR                  0x001A

#define W5100_S3_1K                 0x00
#define W5100_S3_2K                 0x40
#define W5100_S3_4K                 0x80
#define W5100_S3_8K                 0xC0
#define W5100_S3_RMSR_VAL(rmsrVal)  ((rmsrVal & 0xC0) >> 6)

#define W5100_S2_1K                 0x00
#define W5100_S2_2K                 0x10
#define W5100_S2_4K                 0x20
#define W5100_S2_8K                 0x30
#define W5100_S2_RMSR_VAL(rmsrVal)  ((rmsrVal & 0x30) >> 4)

#define W5100_S1_1K                 0x00
#define W5100_S1_2K                 0x04
#define W5100_S1_4K                 0x08
#define W5100_S1_8K                 0x0C
#define W5100_S1_RMSR_VAL(rmsrVal)  ((rmsrVal & 0x0C) >> 2)

#define W5100_S0_1K                 0x00
#define W5100_S0_2K                 0x01
#define W5100_S0_4K                 0x02
#define W5100_S0_8K                 0x03
#define W5100_S0_RMSR_VAL(rmsrVal)  (rmsrVal & 0x03)

// TMSR (TX Memory Size Register) [R/W] [0x001B][0x55]
// This register assigns total 8K TX memory to each socket
// Memory size definitions: use the above
#define W5100_TMSR                  0x001B

#define W5100_S3_TMSR_VAL(tmsrVal)  ((tmsrVal & 0xC0) >> 6)
#define W5100_S2_TMSR_VAL(tmsrVal)  ((tmsrVal & 0x30) >> 4)
#define W5100_S1_TMSR_VAL(tmsrVal)  ((tmsrVal & 0x0C) >> 2)
#define W5100_S0_TMSR_VAL(tmsrVal)  (tmsrVal & 0x03)

// PATR (Authentication Type in PPPoE mode) [R] [0x001C-0x001D][0x0000]
// This register notifies authentication method agreed at connection with PPPoE server
#define W5100_PATR0                 0x001C
#define W5100_PATR1                 0x001D

#define W5100_PAP                   0xC023
#define W5100_CHAP                  0xC223

// PTIMER (PPP Link Control Protocol Request Timer Register) [R/W] [0x0028][0x28]
// This register indicates the duration for sending LCP Echo Request. 1-> 25ms
#define W5100_PTIMER                0x0028

// PMAGIC (PPP Link Control Protocol Magic number Register) [R/W] [0x0029][0x00]
// This register is used in Magic number option during LC negotiation
#define W5100_PMAGIC                0x0029

// UIPR (Unreachable I Address Register) [R] [0x002A-0x002D][0x00]
// In case of data transmission using UD, if transmitting to non existing
// IP address, ICM (Destination Unreachable) packet will be received.
// In this case, that IP address and port number will be saved in the UIPR
// and UPORT registers respectively.
// Example: 192.168.0.11 port 5000 (0x1388)
#define W5100_UIPR0                 0x002A  // 192 (0xC0)
#define W5100_UIPR1                 0x002B  // 168 (0xA8)
#define W5100_UIPR2                 0x002C  // 0   (0x00)
#define W5100_UIPR3                 0x002D  // 11  (0x0B)

// UPORT (Unreachable Port Register) [R] [0x002E-0x002F][0x0000]
// See description above
#define W5100_UPORT0                0x002E  // 0x13
#define W5100_UPORT1                0x002F  // 0x88


////////////////////////////////////////////////////////////////////////////////
// Memory Map - Socket registers

// Sn_MR (Socket n Mode Register) [R/W] [0x0400, 0x0500, 0x0600, 0x0700][0x00]
// This register sets up socket option or protocol for each socket
#define W5100_S0_MR                 0x0400
#define W5100_S1_MR                 0x0500
#define W5100_S2_MR                 0x0600
#define W5100_S3_MR                 0x0700

#define W5100_MULTI                 0x80    // Multicasting (UDP only) (1:enable; 0:disable)
#define W5100_MF                    0x40    // MAC Filter (S0 support only, MACRAW mode only) (1:enable; 0:disable)
#define W5100_ND                    0x20    // Use No Delayed ACK (TCP only) (1:enable; 0:disable)
#define W5100_MC                    0x20    // Multicast (UDP only) (1:use IGMP v1; 0:use IGM v2)
#define W5100_P3                    0x08    //  -
#define W5100_P2                    0x04    //   |  Protocol
#define W5100_P1                    0x02    //   |  (see below)
#define W5100_P0                    0x01    //  -

#define W5100_PROTOCOL_CLOSED       0x00    // P3:0 P2:0 P1:0 P0:0
#define W5100_PROTOCOL_TCP          0x01    // P3:0 P2:0 P1:0 P0:1
#define W5100_PROTOCOL_UDP          0x02    // P3:0 P2:0 P1:1 P0:0
#define W5100_PROTOCOL_IPRAW        0x03    // P3:0 P2:0 P1:1 P0:1
#define W5100_PROTOCOL_MACRAW       0x04    // P3:0 P2:1 P1:0 P0:0 (S0 only)
#define W5100_PROTOCOL_PPPoE        0x05    // P3:0 P2:1 P1:0 P0:1 (S0 only)

// Sn_CR (Socket n Command Register) [R/W] [0x0401, 0x0501, 0x0601, 0x0701][0x00]
// This register is used for socket operation. After performing the command,
// the value will be cleared to 0x00
#define W5100_S0_CR                 0x0401
#define W5100_S1_CR                 0x0501
#define W5100_S2_CR                 0x0601
#define W5100_S3_CR                 0x0701

#define W5100_COMMAND_OPEN          0x01
#define W5100_COMMAND_LISTEN        0x02    // TCP only
#define W5100_COMMAND_CONNECT       0x04    // TCP only
#define W5100_COMMAND_DISCON        0x08    // TCP only
#define W5100_COMMAND_CLOSE         0x10
#define W5100_COMMAND_SEND          0x20
#define W5100_COMMAND_SEND_MAC      0x21    // UDP only
#define W5100_COMMAND_SEND_KEEP     0x22    // TCP only
#define W5100_COMMAND_RECV          0x40

// Sn_IR (Socket n Interrupt Register) [R] [0x0402, 0x0502, 0x0602, 0x0702][0x00]
// This register is used to notify socket connection events
// Socket interrupt register flags must be cleared by writing 1
#define W5100_S0_IR                 0x0402
#define W5100_S1_IR                 0x0502
#define W5100_S2_IR                 0x0602
#define W5100_S3_IR                 0x0702

#define W5100_IR_SEND_OK            0x10
#define W5100_IR_TIMEOUT            0x08
#define W5100_IR_RECV               0x04
#define W5100_IR_DISCON             0x02
#define W5100_IR_CON                0x01

// Sn_SR (Socket n Status Register) [R] [0x0403, 0x0503, 0x0603, 0x0703][0x00]
// This register has the status value of socket n
#define W5100_S0_SR                 0x0403
#define W5100_S1_SR                 0x0503
#define W5100_S2_SR                 0x0603
#define W5100_S3_SR                 0x0703

#define W5100_SOCK_CLOSED           0x00
#define W5100_SOCK_INIT             0x13
#define W5100_SOCK_LISTEN           0x14
#define W5100_SOCK_ESTABLISHED      0x17
#define W5100_SOCK_CLOSE_WAIT       0x1C
#define W5100_SOCK_UDP              0x22
#define W5100_SOCK_IPRAW            0x32
#define W5100_SOCK_MACRAW           0x42
#define W5100_SOCK_PPPOE            0x5F
#define W5100_SOCK_SYNSENT          0x15    // Transient --> to SOCK_ESTABLISHED
#define W5100_SOCK_SYNRECV          0x16    // Transient --> to SOCK_ESTABLISHED
#define W5100_SOCK_FIN_WAIT         0x18    // Transient --> to SOCK_CLOSED
#define W5100_SOCK_CLOSING          0x1A    // Transient --> to SOCK_CLOSED
#define W5100_SOCK_TIME_WAIT        0x1B    // Transient --> to SOCK_CLOSED
#define W5100_SOCK_LAST_ACK         0x1D    // Transient --> to SOCK_CLOSED
#define W5100_SOCK_ARP              0x01    // Transient --> to SOCK_SYNSENT, SOCK_UDP or SOCK_ICMP
#define W5100_SOCK_UNDEFINED        0xFF    // Not in specs

// Sn_PORT (Socket n Source Port) [R/W] [0x0404-0x0405, 0x0504-0x0505, 0x0604-0x0605, 0x0704-0x0705][0x00]
// This register sets the Source Port number for each socket.
// Must be set before executing the OPEN command
// Example: socket 0 port 5000 (0x1388)
#define W5100_S0_PORT0              0x0404  // 0x13
#define W5100_S0_PORT1              0x0405  // 0x88

#define W5100_S1_PORT0              0x0504
#define W5100_S1_PORT1              0x0505

#define W5100_S2_PORT0              0x0604
#define W5100_S2_PORT1              0x0605

#define W5100_S3_PORT0              0x0704
#define W5100_S3_PORT1              0x0705

// Sn_DHAR (Socket n Destination Hardware Address Register) [R/W] [0x0406-0x040B, 0x0506-0x050B, 0x0606-0x060B, 0x0706-0x070B][0xFF]
// This register sets the Destination Hardware Address of each socket
// Example: socket 0 destination hardware address 08.DC.00.01.02.10
#define W5100_S0_DHAR0              0x0406  // 0x08
#define W5100_S0_DHAR1              0x0407  // 0xDC
#define W5100_S0_DHAR2              0x0408  // 0x00
#define W5100_S0_DHAR3              0x0409  // 0x01
#define W5100_S0_DHAR4              0x040A  // 0x02
#define W5100_S0_DHAR5              0x040B  // 0x10

#define W5100_S1_DHAR0              0x0506
#define W5100_S1_DHAR1              0x0507
#define W5100_S1_DHAR2              0x0508
#define W5100_S1_DHAR3              0x0509
#define W5100_S1_DHAR4              0x050A
#define W5100_S1_DHAR5              0x050B

#define W5100_S2_DHAR0              0x0606
#define W5100_S2_DHAR1              0x0607
#define W5100_S2_DHAR2              0x0608
#define W5100_S2_DHAR3              0x0609
#define W5100_S2_DHAR4              0x060A
#define W5100_S2_DHAR5              0x060B

#define W5100_S3_DHAR0              0x0706
#define W5100_S3_DHAR1              0x0707
#define W5100_S3_DHAR2              0x0708
#define W5100_S3_DHAR3              0x0709
#define W5100_S3_DHAR4              0x070A
#define W5100_S3_DHAR5              0x070B

// Sn_DIPR (Socket n Destination IP Address Register) [R/W] [0x040c-0x040F, 0x050C-0x050F, 0x060C-0x060F, 0x070C-0x070F][0x00]
// This register sets the Destination IP Address of each socket
// Example: socket 0 destination IP address 192.168.0.11
#define W5100_S0_DIPR0              0x040C  // 192 (0xC0)
#define W5100_S0_DIPR1              0x040D  // 168 (0xA8)
#define W5100_S0_DIPR2              0x040E  // 0   (0x00)
#define W5100_S0_DIPR3              0x040F  // 11  (0x0B)

#define W5100_S1_DIPR0              0x050C
#define W5100_S1_DIPR1              0x050D
#define W5100_S1_DIPR2              0x050E
#define W5100_S1_DIPR3              0x050F

#define W5100_S2_DIPR0              0x060C
#define W5100_S2_DIPR1              0x060D
#define W5100_S2_DIPR2              0x060E
#define W5100_S2_DIPR3              0x060F

#define W5100_S3_DIPR0              0x070C
#define W5100_S3_DIPR1              0x070D
#define W5100_S3_DIPR2              0x070E
#define W5100_S3_DIPR3              0x070F

// Sn_DPORT (Socket n Destination Port) [R/W] [0x0410-0x0411, 0x0510-0x0511, 0x0610-0x0611, 0x0710-0x0711][0x00]
// This register sets the Destinaiton Port number for each socket.
// In active mode it must be set before executing the CONNECT command; in passive mode, automatically updated
// Example: socket 0 port 5000 (0x1388)
#define W5100_S0_DPORT0             0x0410  // 0x13
#define W5100_S0_DPORT1             0x0411  // 0x88

#define W5100_S1_DPORT0             0x0510
#define W5100_S1_DPORT1             0x0511

#define W5100_S2_DPORT0             0x0610
#define W5100_S2_DPORT1             0x0611

#define W5100_S3_DPORT0             0x0710
#define W5100_S3_DPORT1             0x0711

// Sn_MSS (Socket n Maximum Segment Size Register) [R/W] [0x0412-0x0413, 0x0512-0x0513, 0x0612-0x0613, 0x0712-0x0713][0x0000]
// This register is used for maximum segment size of TCP. In passive mode, automatically set.
// Example: socket 0 maximum segment size 1460 (0x05B4)
#define W5100_S0_MSSR0              0x0412  // 0x05
#define W5100_S0_MSSR1              0x0413  // 0xB4

#define W5100_S1_MSSR0              0x0512
#define W5100_S1_MSSR1              0x0513

#define W5100_S2_MSSR0              0x0612
#define W5100_S2_MSSR1              0x0613

#define W5100_S3_MSSR0              0x0712
#define W5100_S3_MSSR1              0x0713

// Sn_PROTO (Socket n IP Protocol Register) [R/W] [0x0414, 0x0514, 0x0614, 0x0714][0x00]
// This register is used to set up the Protocol field of IP header at IP level in RAW mode
// Refer to IANA online documentation for the list of upperlevel protocol identification number
// Example: Internet Control Message Protocol (ICMP) = 0x01
#define W5100_S0_PROTO              0x0414
#define W5100_S1_PROTO              0x0514
#define W5100_S2_PROTO              0x0614
#define W5100_S3_PROTO              0x0714

// Sn_TOS (Socket n IP Type of Service Register) [R/W] [0x0415, 0x0515, 0x0615, 0x0715][0x00]
// This register is used to set up the Type of Service field of IP header.
#define W5100_S0_TOS                0x0415
#define W5100_S1_TOS                0x0515
#define W5100_S2_TOS                0x0615
#define W5100_S3_TOS                0x0715

// Sn_TTL (Socket n IP Time to Live Register) [R/W] [0x0416, 0x0516, 0x0616, 0x0716][0x80]
// This register is used to set up the Time to Live field of IP header.
#define W5100_S0_TTL                0x0416
#define W5100_S1_TTL                0x0516
#define W5100_S2_TTL                0x0616
#define W5100_S3_TTL                0x0716

// Sn_TX_FSR (Socket n TX Free Size Register) [R] [0x0420-0x0421, 0x0520-0x0521, 0x0620-0x0621, 0x0720-0x0721][0x0800]
// This register show the size of data that the user can transmit. Its value is automatically calculated using
// the values of Sn_TX_RD and Sn_TX_WR.
// When reading this register, read MSB first (0x0420, 0x0520, 0x0620, 0x0720), then read LSB (0x0421, 0x0521, 0x0621, 0x0721).
// Example: free tramsmit size = 2048 (0x0800) bytes
#define W5100_S0_TX_FSR0            0x0420  // 0x08
#define W5100_S0_TX_FSR1            0x0421  // 0x00

#define W5100_S1_TX_FSR0            0x0520
#define W5100_S1_TX_FSR1            0x0521

#define W5100_S2_TX_FSR0            0x0620
#define W5100_S2_TX_FSR1            0x0621

#define W5100_S3_TX_FSR0            0x0720
#define W5100_S3_TX_FSR1            0x0721

// Sn_TX_RD (Socket n TX Read Pointer Register) [R] [0x0422-0x0423, 0x0522-0x0523, 0x0622-0x0623, 0x0722-0x0723][0x0000]
// This register shows the offset in Socket n's TX memory of the last byte transmitted.
// When the SEND command is issued in Sn_CR register, the bytes from the offset stored in Sn_TX_RD to the offset
// stored in Sn_TX_WR are sent, with overflow.
// After transmission is finished, either successfully or with error (both events are signalled in Sn_IR, see description),
// the value of Sn_TX_RD is automatically updated. In case of success, Sn_TX_RD and Sn_TX_WR will have the same value
// after completion of transmission.
// When reading this register, read MSB first (0x0422, 0x0522, 0x0622, 0x0722), then read LSB (0x0423, 0x0523, 0x0623, 0x0723).
// Example: Read Pointer = 2048 (0x0800)
#define W5100_S0_TX_RD0             0x0422  // 0x08
#define W5100_S0_TX_RD1             0x0423  // 0x00

#define W5100_S1_TX_RD0             0x0522
#define W5100_S1_TX_RD1             0x0523

#define W5100_S2_TX_RD0             0x0622
#define W5100_S2_TX_RD1             0x0623

#define W5100_S3_TX_RD0             0x0722
#define W5100_S3_TX_RD1             0x0723

// Sn_TX_WR (Socket n TX Write Pointer Register) [R/W] [0x0424-0x0425, 0x0524-0x0525, 0x0624-0x0625, 0x0724-0x0725][0x0000]
// This register shows the offset in Socket n's TX memory where the first byte to be transmitted can be written.
// When reading this register, read MSB first (0x0424, 0x0524, 0x0624, 0x0724), then read LSB (0x0425, 0x0525, 0x0625, 0x0725).
// When writing this register, write MSB first (0x0424, 0x0524, 0x0624, 0x0724), then write LSB (0x0425, 0x0525, 0x0625, 0x0725).
//
// The physical address where bytes to be sent shall be written is given by adding the Socket n's TX memory base address
// to the value of this register, masked with the Socket n' TX memory size, as follows:
//
// * The Socket n's TX memory base address can be calculated as follows:
//
//   a) Compute the TX memory size of each socket:
//        static const uint16_t oneKB           = 0x0400;         // 1024 bytes
//        uint16_t Sn_TX_memSize = oneK << Sn_TMSR_VALUE;         // for each Sn; see above the definition of Sn_TMSR_VALUE
//
//   b) Compute the base address for each socket
//        uint16_t  S0_TX_memBase = W5100_MEM_TX_BASE;
//        uint16_t  S1_TX_memBase = S0_TX_memBase + S0_TX_memSize;
//        uint16_t  S2_TX_memBase = S1_TX_memBase + S1_TX_memSize;
//        uint16_t  S3_TX_memBase = S2_TX_memBase + S2_TX_memSize;
//
// * Given the Sn_TX_memBase, add the value stored Sn_TX_WR as follows:
//        uint16_t Sn_TX_sizeMask = Sn_TX_memSize - 1;            // For example, for memSize = 2K (0x0800), it gives 0x07FF
//        uint16_t Sn_TX_wrAddr   = Sn_TX_memBase + (read16(Sn_TX_WR) & Sn_TX_sizeMask);
//
// Sn_TX_FSR shows howmany bytes can be safely written in Socket n's TX buffer, starting from Sn_TX_wrAddr. In other words,
// the value read from Sn_TX_FSR indicates the distance from Sn_TX_WR to Sn_TX_RR in the Socket n's TX (ring) buffer.
// If the amount of bytes to be sent (i.e. written in TX buffer) is such that the upper limit of the TX buffer
// is exceeded, the writing shall restart from Sn_TX_memBase and go up until no more bytes need to be written.
//
// After having completed the writing of all bytes into the TX buffer, the value of Sn_TX_WR shall be increased
// by the amount of bytes written, without caring of possible buffer overflows, if any.
// Then, COMMAND_SEND shall be issued to Sn_CR.
// After transmission is successfully finished (this event is signalled in Sn_IR, see description),
// the value of Sn_TX_WR is automatically updated.
// If an error occurs, ???
//
// Example: Write Pointer = 2048 (0x0800)
#define W5100_S0_TX_WR0             0x0424  // 0x08
#define W5100_S0_TX_WR1             0x0425  // 0x00

#define W5100_S1_TX_WR0             0x0524
#define W5100_S1_TX_WR1             0x0525

#define W5100_S2_TX_WR0             0x0624
#define W5100_S2_TX_WR1             0x0625

#define W5100_S3_TX_WR0             0x0724
#define W5100_S3_TX_WR1             0x0725

// Sn_RSR (Socket n Received Size Register) [R]  [0x0426-0x0427, 0x0526-0x0527, 0x0626-0x0627, 0x0726-0x0727][0x0000]
// This register shows the size of data received. Its value is automatically calculated using the values of
// Sn_RX_RD and Sn_RX_WR and is automatically changed by COMMAND_RECV.
// When reading this register, read MSB first (0x0426, 0x0526, 0x0626, 0x0726), then read LSB (0x0427, 0x0527, 0x0627, 0x0727).
// Example: received size = 2048 (0x0800) bytes
#define W5100_S0_RX_RSR0            0x0426  // 0x08
#define W5100_S0_RX_RSR1            0x0427  // 0x00

#define W5100_S1_RX_RSR0            0x0526
#define W5100_S1_RX_RSR1            0x0527

#define W5100_S2_RX_RSR0            0x0626
#define W5100_S2_RX_RSR1            0x0627

#define W5100_S3_RX_RSR0            0x0726
#define W5100_S3_RX_RSR1            0x0727

// Sn_RX_RD (Socket n RX Read Pointer Register) [R/W] [0x0428-0x0429, 0x0528-0x0529, 0x0628-0x0629, 0x0728-0x0729][0x0000]
// This register shows the offset in Socket n's RX memory of the first byte received.
// When the IR_RECV bit is set in Sn_IR register, the amount of bytes indicated in Sn_RX_RSR can be read starting
// from the offset indicated in Sn_RX_RD, with overflow.
// After reception is finished, either successfully or with error (both events are signalled in Sn_IR, see description),
// the value of Sn_RX_RD is automatically updated.
// When reading this register, read MSB first (0x0422, 0x0522, 0x0622, 0x0722), then read LSB (0x0423, 0x0523, 0x0623, 0x0723).
//
// The physical address where received bytes shall be read is given by adding the Socket n' RX memory base address
// to the value of this register, masked with the Socket n' RX memory size, as follows:
//
// * The Socket n's RX memory base address can be calculated as follows:
//
//   a) Compute the RX memory size of each socket:
//        static const uint16_t oneKB           = 0x0400;         // 1024 bytes
//        uint16_t Sn_RX_memSize = oneK << Sn_RMSR_VALUE;         // for each Sn; see above the definition of Sn_RMSR_VALUE
//
//   b) Compute the base address for each socket
//        uint16_t  S0_RX_memBase = W5100_MEM_RX_BASE;
//        uint16_t  S1_RX_memBase = S0_RX_memBase + S0_RX_memSize;
//        uint16_t  S2_RX_memBase = S1_RX_memBase + S1_RX_memSize;
//        uint16_t  S3_RX_memBase = S2_RX_memBase + S2_RX_memSize;
//
// * Given the Sn_RX_memBase, add the value stored Sn_RX_RD as follows:
//        uint16_t Sn_RX_sizeMask = Sn_RX_memSize - 1;            // For example, for memSize = 2K (0x0800), it gives 0x07FF
//        uint16_t Sn_RX_rdAddr   = Sn_RX_memBase + (read16(Sn_RX_RD) & Sn_RX_sizeMask);
//
// Sn_RX_RSR shows howmany bytes can be safely read from Socket n's RX buffer, starting from Sn_RX_rdAddr.
// If the amount of bytes to be read from RX buffer is such that the upper limit of the RX buffer
// is exceeded, the reading shall restart from Sn_RX_memBase and go up until no more bytes need to be read.
//
// After having completed the reading of all bytes from the RX buffer, the value of Sn_RX_RD shall be increased
// by the amount of bytes read, without caring of possible buffer overflows, if any.
// Then, COMMAND_RECV shall be issued to Sn_CR.
//
// Example: Read Pointer = 2048 (0x0800)
#define W5100_S0_RX_RD0             0x0428  // 0x08
#define W5100_S0_RX_RD1             0x0429  // 0x00

#define W5100_S1_RX_RD0             0x0528
#define W5100_S1_RX_RD1             0x0529

#define W5100_S2_RX_RD0             0x0628
#define W5100_S2_RX_RD1             0x0629

#define W5100_S3_RX_RD0             0x0728
#define W5100_S3_RX_RD1             0x0729

////////////////////////////////////////////////////////////////////////////////
// Utility definitions for computation of Socket n register address

#define W5100_HOWMANY_SOCKETS       4

#define W5100_S0_MASK               0x0400
#define W5100_S1_MASK               0x0500
#define W5100_S2_MASK               0x0600
#define W5100_S3_MASK               0x0700

#define W5100_Sn_MR                 0x0000
#define W5100_Sn_CR                 0x0001
#define W5100_Sn_IR                 0x0002
#define W5100_Sn_SR                 0x0003
#define W5100_Sn_PORT               0x0004
#define W5100_Sn_DHAR               0x0006
#define W5100_Sn_DIPR               0x000C
#define W5100_Sn_DPORT              0x0010
#define W5100_Sn_MSSR               0x0012
#define W5100_Sn_PROTO              0x0014
#define W5100_Sn_TOS                0x0015
#define W5100_Sn_TTL                0x0016
#define W5100_Sn_TX_FSR             0x0020
#define W5100_Sn_TX_RD              0x0022
#define W5100_Sn_TX_WR              0x0024
#define W5100_Sn_RX_RSR             0x0026
#define W5100_Sn_RX_RD              0x0028

////////////////////////////////////////////////////////////////////////////////

#endif // #ifndef W5100DEFS_H

