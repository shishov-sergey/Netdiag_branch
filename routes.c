#include "netdiag.h"
#include <netinet/in.h>

/* Преобразование HEX-строки из /proc/net/route в dotted decimal */
void hex_to_ip(unsigned int hex, char *ip_str) {
    unsigned char bytes[4];
    bytes[0] = (hex >> 0) & 0xFF;
    bytes[1] = (hex >> 8) & 0xFF;
    bytes[2] = (hex >> 16) & 0xFF;
    bytes[3] = (hex >> 24) & 0xFF;
    sprintf(ip_str, "%d.%d.%d.%d", bytes[0], bytes[1], bytes[2], bytes[3]);
}

void show_routes(void) {
    FILE *f = fopen("/proc/net/route", "r");
    if (!f) {
        perror("fopen /proc/net/route");
        return;
    }

    char line[MAX_LINE], iface[16];
    unsigned int dest, gateway, mask;
    int flags;

    // Пропускаем заголовок
    fgets(line, sizeof(line), f);

    printf("%-12s %-18s %-18s %-18s %-s\n", "IFACE", "DESTINATION", "GATEWAY", "NETMASK", "FLAGS");
    printf("%-12s %-18s %-18s %-18s %-s\n", "-----", "-----------", "-------", "-------", "-----");

    while (fgets(line, sizeof(line), f)) {
        sscanf(line, "%s %x %x %x %d", iface, &dest, &gateway, &mask, &flags);
        char dest_str[INET_ADDRSTRLEN], gw_str[INET_ADDRSTRLEN], mask_str[INET_ADDRSTRLEN];
        hex_to_ip(dest, dest_str);
        hex_to_ip(gateway, gw_str);
        hex_to_ip(mask, mask_str);

        // Игнорируем маршруты с нулевой маской? Показываем все.
        printf("%-12s %-18s %-18s %-18s 0x%04x\n", iface, dest_str, gw_str, mask_str, flags);
    }
    fclose(f);
}