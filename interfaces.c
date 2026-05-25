#include "netdiag.h"
#include <ctype.h>

/* Чтение строки из файла (обрезает перевод строки) */
int file_read_string(const char *path, char *buf, size_t size) {
	FILE *f = fopen(path, "r");
    if (!f) return -1;
    int ok = fgets(buf, sz, f) != NULL && !ferror(f);
    fclose(f);
    if (!ok) return -1;
    char *nl = strchr(buf, '\n');
    if (nl) *nl = '\0';
    return 0;
}

/* Чтение целого числа из файла */
int file_read_int(const char *path, int *value) {
	char buf[32];
    if (file_read_string(path, buf, sizeof(buf)) != 0) return -1;
    char *end;
    errno = 0;
    long v = strtol(buf, &end, 10);
    if (errno || end == buf || *end != '\0') return -1;
    *val = (int)v;
    return 0;
}

/* Получение IP-адресов интерфейса (IPv4 и IPv6) через getifaddrs */
void get_ip_addresses(const char *iface, char *buffer, size_t buf_size) {
    struct ifaddrs *ifaddr, *ifa;
    char ip_str[INET6_ADDRSTRLEN];
    buffer[0] = '\0';

    if (getifaddrs(&ifaddr) == -1) return;

    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr == NULL) continue;
        if (strcmp(ifa->ifa_name, iface) != 0) continue;

        void *addr;
        int family = ifa->ifa_addr->sa_family;
        if (family == AF_INET) {
            addr = &((struct sockaddr_in*)ifa->ifa_addr)->sin_addr;
            inet_ntop(AF_INET, addr, ip_str, sizeof(ip_str));
        } else if (family == AF_INET6) {
            addr = &((struct sockaddr_in6*)ifa->ifa_addr)->sin6_addr;
            inet_ntop(AF_INET6, addr, ip_str, sizeof(ip_str));
        } else {
            continue;
        }
        if (strlen(buffer) + strlen(ip_str) + 2 < buf_size) {
            if (buffer[0] != '\0') strcat(buffer, ", ");
            strcat(buffer, ip_str);
        }
    }
    freeifaddrs(ifaddr);
}

/* Вывод таблицы интерфейсов */
void show_interfaces(void) {
    DIR *d;
    struct dirent *dir;
    char path[MAX_PATH], state[32], mac[32], ip_list[256];
    int mtu;

    fprintf(stdout,"%-12s %-12s %-20s %-8s %-30s\n", "IFACE", "STATE", "MAC", "MTU", "IP_ADDRESSES");
    fprintf(stdout,"%-12s %-12s %-20s %-8s %-30s\n", "-----", "-----", "---", "---", "------------");

    d = opendir("/sys/class/net");
    if (!d) {
        perror("opendir /sys/class/net");
        return;
    }

    while ((dir = readdir(d)) != NULL) {
        if (dir->d_name[0] == '.') continue;

        snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", dir->d_name);
        if (file_read_string(path, state, sizeof(state)) != 0) strcpy(state, "unknown");

        snprintf(path, sizeof(path), "/sys/class/net/%s/address", dir->d_name);
        if (file_read_string(path, mac, sizeof(mac)) != 0) strcpy(mac, "00:00:00:00:00:00");

        snprintf(path, sizeof(path), "/sys/class/net/%s/mtu", dir->d_name);
        if (file_read_int(path, &mtu) != 0) mtu = 0;

        get_ip_addresses(dir->d_name, ip_list, sizeof(ip_list));
        if (strlen(ip_list) == 0) strcpy(ip_list, "-");

        fprintf(stdout, "%-12s %-12s %-20s %-8d %-30s\n", dir->d_name, state, mac, mtu, ips);
    }
    closedir(d);
}