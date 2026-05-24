# netdiag - Network Diagnostic Tool

A CLI utility for network diagnostics on Linux systems, written in C.

## Features

- **Show Interfaces** - Display all network interfaces with their state, MAC, MTU, and IP addresses
- **Show Routes** - Display the routing table
- **Show VLANs** - Display VLAN interfaces
- **Check Link** - Check link status, state, and perform ping test for a specific interface
- **Check Gateway** - Check gateway reachability and display route information
- **Collect Diagnostics** - Create a comprehensive diagnostic archive

## Requirements

- Linux operating system
- GCC compiler
- Standard Linux networking tools (ip, ping, netstat, arp, lspci, dmesg, tar)

## Building

```bash
make