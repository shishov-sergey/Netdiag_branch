#include "netdiag.h"
#include <sys/wait.h>

void check_link(const char *iface) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", iface);
    char state[32];
    if (file_read_string(path, state, sizeof(state)) != 0) {
        printf("Interface '%s' does not exist or cannot be read.\n", iface);
        return;
    }

    printf("Interface: %s\n", iface);
    printf("State: %s\n", state);

    // Дополнительно проверим carrier
    snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", iface);
    int carrier;
    if (file_read_int(path, &carrier) == 0) {
        printf("Carrier: %s\n", carrier ? "YES (cable connected)" : "NO (cable disconnected)");
    }

    // Скорость (если интерфейс UP)
    if (strcmp(state, "up") == 0) {
        snprintf(path, sizeof(path), "/sys/class/net/%s/speed", iface);
        int speed;
        if (file_read_int(path, &speed) == 0 && speed > 0) {
            printf("Speed: %d Mbps\n", speed);
        }
    }
}

void check_gateway(const char *ip) {
    char cmd[MAX_LINE];
    snprintf(cmd, sizeof(cmd), "ping -c 2 -W 2 %s > /tmp/netdiag_ping.tmp 2>&1", ip);
    int ret = system(cmd);

    FILE *f = fopen("/tmp/netdiag_ping.tmp", "r");
    if (!f) {
        printf("Failed to execute ping.\n");
        return;
    }

    char line[MAX_LINE];
    int received = 0;
    float loss = 100.0;
    float rtt_min = 0, rtt_avg = 0, rtt_max = 0;

    while (fgets(line, sizeof(line), f)) {
        if (strstr(line, "received")) {
            sscanf(line, "%*d packets transmitted, %d received", &received);
            int transmitted = 0;
            sscanf(line, "%d packets transmitted", &transmitted);
            if (transmitted > 0) loss = 100.0 - (received * 100.0 / transmitted);
        }
        if (strstr(line, "rtt min/avg/max")) {
            sscanf(line, "%*s %*s %f/%f/%f", &rtt_min, &rtt_avg, &rtt_max);
        }
    }
    fclose(f);
    remove("/tmp/netdiag_ping.tmp");

    printf("Gateway: %s\n", ip);
    printf("Ping result: %d packets received, %.1f%% loss\n", received, loss);
    if (rtt_avg > 0) {
        printf("RTT (min/avg/max) = %.3f / %.3f / %.3f ms\n", rtt_min, rtt_avg, rtt_max);
    }
}