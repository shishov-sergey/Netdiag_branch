#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <dirent.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if_arp.h>
#include <linux/sockios.h>
#include <errno.h>

#define MAX_IFACES 32
#define MAX_IP_ADDRS 16
#define BUFFER_SIZE 4096

typedef struct {
    char name[16];
    char state[10];
    char mac[18];
    int mtu;
    char ip_addrs[MAX_IP_ADDRS][20];
    int ip_count;
} network_interface;

// Функция для чтения состояния интерфейса из /sys/class/net
void get_interface_state(const char *iface, char *state) {
    char path[256];
    FILE *fp;
    char operstate[32];
    
    snprintf(path, sizeof(path), "/sys/class/net/%s/operstate", iface);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%s", operstate);
        fclose(fp);
        
        if (strcmp(operstate, "up") == 0) {
            strcpy(state, "UP");
        } else if (strcmp(operstate, "down") == 0) {
            strcpy(state, "DOWN");
        } else {
            strcpy(state, operstate);
        }
    } else {
        strcpy(state, "UNKNOWN");
    }
}

// Функция для получения MAC-адреса через sysfs
void get_mac_address(const char *iface, char *mac) {
    char path[256];
    FILE *fp;
    unsigned char addr[6];
    
    snprintf(path, sizeof(path), "/sys/class/net/%s/address", iface);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%s", mac);
        fclose(fp);
    } else {
        strcpy(mac, "00:00:00:00:00:00");
    }
}

// Функция для получения MTU через sysfs
int get_mtu(const char *iface) {
    char path[256];
    FILE *fp;
    int mtu;
    
    snprintf(path, sizeof(path), "/sys/class/net/%s/mtu", iface);
    fp = fopen(path, "r");
    if (fp) {
        fscanf(fp, "%d", &mtu);
        fclose(fp);
        return mtu;
    }
    return 1500;
}

// Функция для получения IP-адресов через ioctl
void get_ip_addresses(const char *iface, char ip_addrs[MAX_IP_ADDRS][20], int *ip_count) {
    int sock;
    struct ifreq ifr;
    struct sockaddr_in *addr;
    
    *ip_count = 0;
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) return;
    
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    
    if (ioctl(sock, SIOCGIFADDR, &ifr) == 0) {
        addr = (struct sockaddr_in *)&ifr.ifr_addr;
        strcpy(ip_addrs[*ip_count], inet_ntoa(addr->sin_addr));
        (*ip_count)++;
    }
    
    close(sock);
}

// Команда: show interfaces
void show_interfaces() {
    DIR *dir;
    struct dirent *entry;
    network_interface interfaces[MAX_IFACES];
    int iface_count = 0;
    
    dir = opendir("/sys/class/net");
    if (!dir) {
        fprintf(stderr, "Error: Cannot open /sys/class/net\n");
        return;
    }
    
    while ((entry = readdir(dir)) != NULL && iface_count < MAX_IFACES) {
        if (entry->d_name[0] == '.') continue;
        
        strcpy(interfaces[iface_count].name, entry->d_name);
        get_interface_state(entry->d_name, interfaces[iface_count].state);
        get_mac_address(entry->d_name, interfaces[iface_count].mac);
        interfaces[iface_count].mtu = get_mtu(entry->d_name);
        get_ip_addresses(entry->d_name, interfaces[iface_count].ip_addrs, 
                        &interfaces[iface_count].ip_count);
        iface_count++;
    }
    closedir(dir);
    
    // Вывод таблицы
    printf("\n%-15s %-12s %-20s %-8s %s\n", 
           "IFACE", "STATE", "MAC", "MTU", "IP ADDRESSES");
    printf("%-15s %-12s %-20s %-8s %s\n", 
           "-----", "-----", "---", "---", "------------");
    
    for (int i = 0; i < iface_count; i++) {
        printf("%-15s %-12s %-20s %-8d", 
               interfaces[i].name, 
               interfaces[i].state,
               interfaces[i].mac,
               interfaces[i].mtu);
        
        if (interfaces[i].ip_count > 0) {
            printf(" %s", interfaces[i].ip_addrs[0]);
            for (int j = 1; j < interfaces[i].ip_count; j++) {
                printf(", %s", interfaces[i].ip_addrs[j]);
            }
        }
        printf("\n");
    }
    printf("\n");
}

// Команда: show routes
void show_routes() {
    printf("\n=== Routing Table ===\n\n");
    system("ip route show");
    printf("\n");
}

// Команда: show vlans
void show_vlans() {
    printf("\n=== VLAN Interfaces ===\n\n");
    
    DIR *dir;
    struct dirent *entry;
    int vlan_count = 0;
    
    dir = opendir("/proc/net/vlan");
    if (dir) {
        while ((entry = readdir(dir)) != NULL) {
            if (entry->d_name[0] != '.') {
                printf("  %s\n", entry->d_name);
                vlan_count++;
            }
        }
        closedir(dir);
    }
    
    if (vlan_count == 0) {
        printf("  No VLAN interfaces found\n");
    }
    
    // Альтернативный метод через ip命令
    printf("\n  Detailed VLAN info (via ip command):\n");
    system("ip -d link show | grep -A 1 vlan | head -20");
    printf("\n");
}

// Команда: check link
int check_link(const char *iface) {
    int sock;
    struct ifreq ifr;
    
    printf("\n=== Checking link for %s ===\n\n", iface);
    
    sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }
    
    strncpy(ifr.ifr_name, iface, IFNAMSIZ - 1);
    
    // Проверка состояния интерфейса
    char state[32];
    get_interface_state(iface, state);
    printf("Interface state: %s\n", state);
    
    // Получение MAC-адреса
    char mac[18];
    get_mac_address(iface, mac);
    printf("MAC address: %s\n", mac);
    
    // Получение MTU
    int mtu = get_mtu(iface);
    printf("MTU: %d\n", mtu);
    
    // Проверка флагов интерфейса
    if (ioctl(sock, SIOCGIFFLAGS, &ifr) == 0) {
        printf("Flags: 0x%x\n", ifr.ifr_flags);
        printf("Link status: %s\n", 
               (ifr.ifr_flags & IFF_RUNNING) ? "UP" : "DOWN");
    }
    
    close(sock);
    
    // Проверка связи через ping
    printf("\nPerforming ping test...\n");
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ping -c 3 -I %s 8.8.8.8 2>/dev/null | tail -3", iface);
    system(cmd);
    
    printf("\n");
    return 0;
}

// Команда: check gateway
int check_gateway(const char *gateway_ip) {
    printf("\n=== Checking gateway %s ===\n\n", gateway_ip);
    
    // Пинг шлюза
    printf("Pinging gateway %s...\n", gateway_ip);
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "ping -c 4 %s", gateway_ip);
    int result = system(cmd);
    
    if (result == 0) {
        printf("\n✓ Gateway %s is reachable\n", gateway_ip);
    } else {
        printf("\n✗ Gateway %s is NOT reachable\n", gateway_ip);
    }
    
    // Маршрут до шлюза
    printf("\nRoute to gateway:\n");
    snprintf(cmd, sizeof(cmd), "ip route get %s", gateway_ip);
    system(cmd);
    
    printf("\n");
    return result;
}

// Команда: collect - сбор диагностического архива
void collect_diagnostics() {
    char timestamp[64];
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, sizeof(timestamp), "%Y%m%d_%H%M%S", t);
    
    char dirname[128];
    snprintf(dirname, sizeof(dirname), "netdiag_collect_%s", timestamp);
    
    printf("\n=== Collecting Diagnostic Information ===\n\n");
    printf("Creating directory: %s\n", dirname);
    
    if (mkdir(dirname, 0755) != 0) {
        perror("mkdir");
        return;
    }
    
    // Сбор информации
    struct {
        const char *cmd;
        const char *output_file;
    } commands[] = {
        {"ip addr show", "ip_addr.txt"},
        {"ip route show", "ip_route.txt"},
        {"ip link show", "ip_link.txt"},
        {"netstat -i", "netstat_interfaces.txt"},
        {"arp -n", "arp_cache.txt"},
        {"cat /proc/net/dev", "proc_net_dev.txt"},
        {"cat /proc/net/route", "proc_net_route.txt"},
        {"lspci | grep -i ethernet", "pci_ethernet.txt"},
        {"dmesg | grep -i eth | tail -50", "dmesg_eth.txt"},
        {"cat /etc/network/interfaces 2>/dev/null", "interfaces_config.txt"},
    };
    
    int num_commands = sizeof(commands) / sizeof(commands[0]);
    
    for (int i = 0; i < num_commands; i++) {
        char full_path[256];
        snprintf(full_path, sizeof(full_path), "%s/%s", dirname, commands[i].output_file);
        
        char cmd[512];
        snprintf(cmd, sizeof(cmd), "%s > %s 2>&1", commands[i].cmd, full_path);
        system(cmd);
        
        printf("  ✓ Collected: %s\n", commands[i].output_file);
    }
    
    // Сбор информации об интерфейсах через sysfs
    char sysfs_dir[256];
    snprintf(sysfs_dir, sizeof(sysfs_dir), "%s/sysfs_net", dirname);
    mkdir(sysfs_dir, 0755);
    
    DIR *dirp = opendir("/sys/class/net");
    if (dirp) {
        struct dirent *entry;
        while ((entry = readdir(dirp)) != NULL) {
            if (entry->d_name[0] == '.') continue;
            
            char iface_dir[256];
            snprintf(iface_dir, sizeof(iface_dir), "%s/%s", sysfs_dir, entry->d_name);
            mkdir(iface_dir, 0755);
            
            const char *files[] = {"operstate", "mtu", "address", "speed", "duplex"};
            for (int i = 0; i < 5; i++) {
                char src_path[256];
                char dst_path[256];
                snprintf(src_path, sizeof(src_path), "/sys/class/net/%s/%s", 
                        entry->d_name, files[i]);
                snprintf(dst_path, sizeof(dst_path), "%s/%s", iface_dir, files[i]);
                
                char cmd[512];
                snprintf(cmd, sizeof(cmd), "cp %s %s 2>/dev/null", src_path, dst_path);
                system(cmd);
            }
        }
        closedir(dirp);
        printf("  ✓ Collected sysfs interface information\n");
    }
    
    // Создание архива
    char archive_name[256];
    snprintf(archive_name, sizeof(archive_name), "%s.tar.gz", dirname);
    
    printf("\nCreating archive: %s\n", archive_name);
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "tar -czf %s %s/", archive_name, dirname);
    system(cmd);
    
    printf("\n✓ Diagnostic collection complete!\n");
    printf("  Archive: %s\n", archive_name);
    printf("  Directory: %s/\n", dirname);
    printf("\nTo extract: tar -xzf %s\n", archive_name);
    printf("\n");
}

void print_usage() {
    printf("Usage: netdiag <command> [arguments]\n\n");
    printf("Commands:\n");
    printf("  show interfaces           - Show all network interfaces\n");
    printf("  show routes               - Show routing table\n");
    printf("  show vlans                - Show VLAN interfaces\n");
    printf("  check link <interface>    - Check link status for interface\n");
    printf("  check gateway <ip>        - Check gateway reachability\n");
    printf("  collect                   - Collect diagnostic archive\n\n");
    printf("Examples:\n");
    printf("  ./netdiag show interfaces\n");
    printf("  ./netdiag check link eth0\n");
    printf("  ./netdiag check gateway 192.168.1.1\n");
    printf("  ./netdiag collect\n\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage();
        return 1;
    }
    
    if (strcmp(argv[1], "show") == 0) {
        if (argc < 3) {
            printf("Error: show requires a subcommand\n");
            print_usage();
            return 1;
        }
        
        if (strcmp(argv[2], "interfaces") == 0) {
            show_interfaces();
        } else if (strcmp(argv[2], "routes") == 0) {
            show_routes();
        } else if (strcmp(argv[2], "vlans") == 0) {
            show_vlans();
        } else {
            printf("Unknown show subcommand: %s\n", argv[2]);
            print_usage();
            return 1;
        }
    } 
    else if (strcmp(argv[1], "check") == 0) {
        if (argc < 4) {
            printf("Error: check requires a subcommand and argument\n");
            print_usage();
            return 1;
        }
        
        if (strcmp(argv[2], "link") == 0) {
            check_link(argv[3]);
        } else if (strcmp(argv[2], "gateway") == 0) {
            check_gateway(argv[3]);
        } else {
            printf("Unknown check subcommand: %s\n", argv[2]);
            print_usage();
            return 1;
        }
    }
    else if (strcmp(argv[1], "collect") == 0) {
        collect_diagnostics();
    }
    else {
        printf("Unknown command: %s\n", argv[1]);
        print_usage();
        return 1;
    }
    
    return 0;
}