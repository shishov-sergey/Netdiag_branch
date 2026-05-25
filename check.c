#include "netdiag.h"
#include <sys/wait.h>

/* Проверка интерфейса линка */
void check_link(const char *iface) {
    char path[MAX_PATH];
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", iface);
    char state[32];
    if (file_read_string(path, state, sizeof(state)) != 0) {
        fprintf(stderr, "Interface '%s' does not exist or cannot be read.\n", iface);
        return;
    }

    fprintf(stdout,"Interface: %s\n", iface);
    fprintf(stdout,"State: %s\n", state);

    // Проверка carrier
    snprintf(path, sizeof(path), "/sys/class/net/%s/carrier", iface);
    int carrier;
    if (file_read_int(path, &carrier) == 0) {
        fprintf(stdout, "Carrier: %s\n", carrier ? "YES (cable connected)" : "NO (cable disconnected)");
    }

    // Скорость 
    if (strcmp(state, "up") == 0) {
        snprintf(path, sizeof(path), "/sys/class/net/%s/speed", iface);
        int speed;
        if (file_read_int(path, &speed) == 0 && speed > 0) {
            fprintf(stdout, "Speed: %d Mbps\n", speed);
        }
    }
}

/* Валидация IPv4 */
static int is_valid_ipv4(const char *ip) {
    unsigned int a,b,c,d; 
    return sscanf(ip, "%u.%u.%u.%u", &a,&b,&c,&d) == 4 && a<256 && b<256 && c<256 && d<256;
}

/* Проверка шлюза: через popen */
void check_gateway(const char *ip) {
	if (!is_valid_ipv4(ip)) { 
	  fprintf(stderr, "Invalid IPv4 address: %s\n", ip); 
	  return; 
	}

    char cmd[128]; 
    snprintf(cmd, sizeof(cmd), "ping -c 2 -W 2 -q %s 2>&1", ip);
    FILE *p = popen(cmd, "r"); 
    if (!p) { 
      fprintf(stderr, "Failed to execute ping.\n"); 
      return; 
    }

    char line[256]; int recv = 0, sent = 0; float loss = 100, rtt[3] = {0};
    while (fgets(line, sizeof(line), p)) {
        if (strstr(line, "received")) 
          sscanf(line, "%d packets transmitted, %d received", &sent, &recv);
        if (strstr(line, "rtt min/avg/max")) 
          sscanf(line, "%*s %*s %f/%f/%f", &rtt[0], &rtt[1], &rtt[2]);
    }
    pclose(p);
    if (sent > 0) 
      loss = 100.0f - (recv * 100.0f / sent);

    fprintf(stdout, "Gateway: %s\nPing: %d/%d packets received, %.1f%% loss\n", ip, recv, sent, loss);
    if (rtt[1] > 0) 
      fprintf(stdout, "RTT (min/avg/max): %.3f/%.3f/%.3f ms\n", rtt[0], rtt[1], rtt[2]);
}
