#include "netdiag.h"

void show_vlans(void) {
    FILE *f = fopen("/proc/net/vlan/config", "r");
    if (!f) {
        printf("No VLAN interfaces found (check if 8021q module is loaded)\n");
        return;
    }

    char line[MAX_LINE];
    printf("VLAN configuration:\n");
    while (fgets(line, sizeof(line), f)) {
        // Убираем лишние пробелы в начале
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (strncmp(p, "VLAN", 4) == 0 || strstr(p, "|")) {
            printf("%s", line);
        }
    }
    fclose(f);
}