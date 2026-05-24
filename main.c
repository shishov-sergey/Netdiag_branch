#include "netdiag.h"

void print_usage(const char *prog) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s show interfaces\n", prog);
    fprintf(stderr, "  %s show routes\n", prog);
    fprintf(stderr, "  %s show vlans\n", prog);
    fprintf(stderr, "  %s check link <interface>\n", prog);
    fprintf(stderr, "  %s check gateway <IP>\n", prog);
    fprintf(stderr, "  %s collect\n", prog);
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    if (strcmp(argv[1], "show") == 0) {
        if (argc < 3) {
            print_usage(argv[0]);
            return 1;
        }
        if (strcmp(argv[2], "interfaces") == 0) {
            show_interfaces();
        } else if (strcmp(argv[2], "routes") == 0) {
            show_routes();
        } else if (strcmp(argv[2], "vlans") == 0) {
            show_vlans();
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } else if (strcmp(argv[1], "check") == 0) {
        if (argc < 4) {
            print_usage(argv[0]);
            return 1;
        }
        if (strcmp(argv[2], "link") == 0) {
            check_link(argv[3]);
        } else if (strcmp(argv[2], "gateway") == 0) {
            check_gateway(argv[3]);
        } else {
            print_usage(argv[0]);
            return 1;
        }
    } else if (strcmp(argv[1], "collect") == 0) {
        collect_diagnostics();
    } else {
        print_usage(argv[0]);
        return 1;
    }

    return 0;
}