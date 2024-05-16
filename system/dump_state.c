#include <dump_state.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>
int dump_file(const char *title, const char *path)
{
    char buffer[32768]; //32kb
    int fd = open(path, O_RDONLY);
    int newline = 0;
    if(fd < 0) {
	int err = errno;
	if(title) {
		printf("--------- %s (%s) --------\n",title, path);
		printf("*** %s: %s\n", path, strerror(err));

		printf("\n");
	}
		return -1;

    }


	if(title) {
		printf("--------- %s (%s",title, path);
		struct stat st;
		if(memcmp(path, "/proc/", 6) != 0 && \
				memcmp(path, "/sys/", 5) != 0\
				&& fstat(fd, &st) == 0) {

		    char stamp[80];
		    time_t mtime = st.st_mtime;
		    struct tm *formatted_time = localtime(&mtime);
		    strftime(stamp, sizeof(stamp), "%Y-%m-%d %H:%M:%S", \
				    formatted_time);
		    printf("%s", stamp);

		}
		printf(") -------\n");
	}

	while(1) {

	    int ret = read(fd, buffer, sizeof(buffer));
	    if (ret > 0) {
		newline = (buffer[ret - 1] == '\n');
		ret = fwrite(buffer, ret, 1, stdout);

	    }
	    if (ret <= 0)    break;

	}

	close(fd);
	if(newline == 0)	printf("\n");
	if(title)    printf("\n");

	return 0;
}





void dump_state(void)
{

    char date[1024];
    time_t raw_time = time(NULL);
    struct tm *formatted_time = localtime(&raw_time);
    strftime(date, sizeof(date), "%Y-%m-%d %H:%M:%S", formatted_time);

    printf("===============================================================\n");
    printf("== dumpstate: %s\n", date);
    printf("===============================================================\n");

    printf("\n");

    printf("Kernel: ");
    dump_file(NULL, "/proc/version");

    printf("\n");

    dump_file("MEMORY INFO", "/proc/meminfo");
    dump_file("VIRTUAL MEMORY STATS", "/proc/vmstat");
    dump_file("VMALLOC INFO", "/proc/vmallocinfo");
    dump_file("SLAB INFO", "/proc/slabinfo");
    dump_file("ZONEINFO", "/proc/zoneinfo");
    dump_file("PAGETYPEINFO", "/proc/pagetypeinfo");
    dump_file("BUDDYINFO", "/proc/buddyinfo");
    dump_file("NETWORK DEV INFO", "/proc/net/dev");
    dump_file("NETWORK ROUTES", "/proc/net/route");
    dump_file("NETWORK ROUTES IPV6", "/proc/net/ipv6_route");
    dump_file("INTERRUPTS (1)", "/proc/interrupts");
    printf("\n");
    printf("========================================================\n");
    printf("== dumpstate: done\n");
    printf("========================================================\n");

}
