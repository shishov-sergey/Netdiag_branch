#include "netdiag.h"

void show_vlans(void) {
    FILE *f = fopen("/proc/net/vlan/config", "r");
    if (!f) {
        fprintf(stderr, "No VLAN interfaces found (check if 8021q module is loaded)\n");
        return;
    }

    char line[MAX_LINE];
    fprintf(stdout, "VLAN configuration:\n");
    while (fgets(line, sizeof(line), f)) {
        char *p = line;
        while (*p == ' ' || *p == '\t') p++;
        if (strncmp(p, "VLAN", 4) == 0 || strstr(p, "|")) {
            fprintf(stdout, "%s", line);
        }
    }
    fclose(f);
}