#ifndef NETDIAG_H
#define NETDIAG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <arpa/inet.h>
#include <time.h>

/* Максимальная длина строки для буферов */
#define MAX_LINE 1024
#define MAX_PATH 256

/* Функции модулей */
void show_interfaces(void);
void show_routes(void);
void show_vlans(void);
void check_link(const char *iface);
void check_gateway(const char *ip);
void collect_diagnostics(void);

/* Вспомогательные функции */
int file_read_string(const char *path, char *buf, size_t size);
int file_read_int(const char *path, int *value);
void get_ip_addresses(const char *iface, char *buffer, size_t buf_size);

#endif