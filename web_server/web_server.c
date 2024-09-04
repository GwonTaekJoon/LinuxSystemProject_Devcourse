#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/mman.h>
#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>
#include <signal.h>
#include <sys/mman.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <net/if.h>
#include <unistd.h>
#include <arpa/inet.h>


#define STACK_SIZE (8 * 1024 * 1024)
#define errExit(msg)    do{perror(msg); exit(EXIT_FAILURE);\
                        } while(0)

static int child_func(void *arg)
{

    char path[1024];
    struct ifreq ifr;
    char *array = "eth0";

    int n = socket(AF_INET, SOCK_DGRAM, 0);
    ifr.ifr_addr.sa_family = AF_INET;
    strncpy(ifr.ifr_name, array, IFNAMSIZ - 1);
    ioctl(n, SIOCGIFADDR, &ifr);
    close(n);

    if(getcwd(path, 1024) == NULL) {
        //pwd
        fprintf(stderr, "current working directory get error: %s\n",\
        strerror(errno));
        return -1;
    }

    printf(" - [%4d] Current namespace Parent PID: %d\n",getpid(), getppid());
    printf("current working directory: %s\n", path);

    /* filebrowser
    if(execl("/usr/local/bin/filebrowser", "filebrowser", "-p", "8282", \
    (char *)NULL)) {
        printf("exec failed\n");
    } */

   if(execl("./toy-be", "toy-be", "-a", inet_ntoa(((struct sockaddr_in *)&ifr.ifr_addr) -> sin_addr), "-p", "8080",\
    "-i", "12341234", (char *)NULL)) {
        printf("execl failed\n");
    }


}
int create_web_server()
{
    pid_t web_server_Pid;

    printf("creating web_server...\n");

/*
    switch (web_server_Pid = fork())
    {
    case -1:
        printf("web_server fork failed\n");
        break;

    case 0:
        if (execl("/usr/local/bin/filebrowser", "filebrowser", "-p", "8284", (char *) NULL)) {
            //int execl(char const *path, char const *arg0, ...);

            printf("web_server exec failed\n");
        }
        break;
    
    
    default:
        break;
    }
*/
    char *stack = mmap(NULL, STACK_SIZE, PROT_READ | PROT_WRITE,\
    MAP_PRIVATE | MAP_ANONYMOUS | MAP_STACK, -1, 0);
    if(stack == MAP_FAILED) {
        printf("mmap failed\n");
        return -1;
    }

    web_server_Pid = clone(child_func, stack + STACK_SIZE, SIGCHLD, "Hello");
    if(web_server_Pid == -1) {
        errExit("clone\n");
    }

    printf("PID of child created by clone() is %ld\n", \
    (long)web_server_Pid);

    munmap(stack, STACK_SIZE);



    return 0;
}
