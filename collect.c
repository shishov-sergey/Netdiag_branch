#include "netdiag.h"
#include <sys/stat.h>

void collect_diagnostics(void) {
    char dirname[64];
    char cmd[MAX_LINE * 2];
    time_t t = time(NULL);
    struct tm *tm = localtime(&t);
    strftime(dirname, sizeof(dirname), "netdiag_%Y%m%d_%H%M%S", tm);

    // Создаём временную директорию в /tmp
    snprintf(cmd, sizeof(cmd), "mkdir -p /tmp/%s", dirname);
    system(cmd);

    // Собираем информацию
    system("echo '=== ip addr ===' > /tmp/$$.tmp");
    system("ip addr >> /tmp/$$.tmp 2>&1");
    system("echo '=== ip route ===' >> /tmp/$$.tmp");
    system("ip route >> /tmp/$$.tmp 2>&1");
    system("echo '=== ip link ===' >> /tmp/$$.tmp");
    system("ip link >> /tmp/$$.tmp 2>&1");
    system("echo '=== arp -n ===' >> /tmp/$$.tmp");
    system("arp -n >> /tmp/$$.tmp 2>&1");
    system("echo '=== dmesg | tail -100 ===' >> /tmp/$$.tmp");
    system("dmesg | tail -100 >> /tmp/$$.tmp 2>&1");
    system("echo '=== netstat -i ===' >> /tmp/$$.tmp");
    system("netstat -i >> /tmp/$$.tmp 2>&1");
    // Копируем файлы из /proc
    snprintf(cmd, sizeof(cmd), "cp /proc/net/route /tmp/%s/ 2>/dev/null", dirname);
    system(cmd);
    snprintf(cmd, sizeof(cmd), "cp /proc/net/vlan/config /tmp/%s/ 2>/dev/null", dirname);
    system(cmd);
    // Перемещаем временный файл с объединённым выводом
    snprintf(cmd, sizeof(cmd), "mv /tmp/$$.tmp /tmp/%s/diagnostics.txt", dirname);
    system(cmd);

    // Создаём архив
    char archive_name[128];
    strftime(archive_name, sizeof(archive_name), "netdiag_%Y%m%d_%H%M%S.tar.gz", tm);
    snprintf(cmd, sizeof(cmd), "cd /tmp && tar czf %s %s", archive_name, dirname);
    system(cmd);

    // Удаляем временную директорию
    snprintf(cmd, sizeof(cmd), "rm -rf /tmp/%s", dirname);
    system(cmd);

    printf("Diagnostic archive created: /tmp/%s\n", archive_name);
    printf("You can remove it manually when no longer needed.\n");
}