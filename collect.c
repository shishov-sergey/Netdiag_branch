#include "netdiag.h"
#include <sys/stat.h>

void collect_diagnostics(void) {
    char dirname[64];
    char tmpfile[256];
    char dest[512];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    
    strftime(dirname, sizeof(dirname), "netdiag_%Y%m%d_%H%M%S", tm);
    snprintf(tmpfile, sizeof(tmpfile), "/tmp/netdiag_%d.tmp", getpid());
    
    // Создаем директорию
    mkdir("/tmp", 0755);
    snprintf(dest, sizeof(dest), "/tmp/%s", dirname);
    mkdir(dest, 0755);
    
    // Открываем файл для записи
    FILE *out = fopen(tmpfile, "w");
    if (!out) return;
    
    // Функция для выполнения команд
    #define RUN(cmd, label) do { \
        fprintf(out, "\n=== %s ===\n", label); \
        FILE *p = popen(cmd, "r"); \
        if (p) { \
            char buf[4096]; \
            while (fgets(buf, sizeof(buf), p)) fputs(buf, out); \
            pclose(p); \
        } else { \
            fprintf(out, "Failed to run: %s\n", cmd); \
        } \
    } while(0)
    
    RUN("ip addr", "ip addr");
    RUN("ip route", "ip route");
    RUN("ip link", "ip link");
    RUN("arp -n", "arp -n");
    RUN("dmesg | tail -100", "dmesg | tail -100");
    RUN("netstat -i", "netstat -i");
    
    #undef RUN
    
    // Чтение файлов /proc напрямую
    fprintf(out, "\n=== /proc/net/route ===\n");
    FILE *proc = fopen("/proc/net/route", "r");
    if (proc) {
        char buf[4096];
        while (fgets(buf, sizeof(buf), proc)) fputs(buf, out);
        fclose(proc);
    }
    
    fprintf(out, "\n=== /proc/net/vlan/config ===\n");
    proc = fopen("/proc/net/vlan/config", "r");
    if (proc) {
        char buf[4096];
        while (fgets(buf, sizeof(buf), proc)) fputs(buf, out);
        fclose(proc);
    }
    
    fclose(out);
    
    // Перемещаем и создаем архив
    char final_path[512];
    snprintf(final_path, sizeof(final_path), "%s/diagnostics.txt", dest);
    rename(tmpfile, final_path);
    
    char archive[128];
    strftime(archive, sizeof(archive), "netdiag_%Y%m%d_%H%M%S.tar.gz", tm);
    
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), "cd /tmp && tar czf %s %s 2>/dev/null", archive, dirname);
    system(cmd);  // tar оставляем, это безопасно
    
    snprintf(cmd, sizeof(cmd), "rm -rf /tmp/%s", dirname);
    system(cmd);
    
    printf("Diagnostic archive created: /tmp/%s\n", archive);
}

