#include "netdiag.h"

int print_usage(const char *prog) {
    fprintf(stderr, "Usage:\n");
    fprintf(stderr, "  %s show interfaces\n", prog);
    fprintf(stderr, "  %s show routes\n", prog);
    fprintf(stderr, "  %s show vlans\n", prog);
    fprintf(stderr, "  %s check link <interface>\n", prog);
    fprintf(stderr, "  %s check gateway <IP>\n", prog);
    fprintf(stderr, "  %s collect\n", prog);
	return 1;
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        return print_usage(argv[0]);
    }

    if (strcmp(argv[1], "show") == 0) {
        if (argc != 3) {
			return print_usage(argv[0]);
		};
        if (strcmp(argv[2], "interfaces") == 0) show_interfaces();
        if (strcmp(argv[2], "routes") == 0)     show_routes();
        if (strcmp(argv[2], "vlans") == 0)      show_vlans();
    } else if (strcmp(argv[1], "check") == 0) {
        if (argc != 4) {
			return print_usage(argv[0]);
		};
        if (strcmp(argv[2], "link") == 0)       check_link(argv[3]);
        if (strcmp(argv[2], "gateway") == 0)    check_gateway(argv[3]);
    } else if (strcmp(argv[1], "collect") == 0) {
        collect_diagnostics();
    }

    return 0;
}
