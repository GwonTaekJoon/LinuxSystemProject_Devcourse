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

#define STACK_SIZE (8 * 1024 * 1024)
#define errExit(msg)    do{perror(msg); exit(EXIT_FAILURE);\
                        } while(0)

static int child_func(void *arg)
{

    char path[1024];

    if(getcwd(path, 1024) == NULL) {
        //pwd
        fprintf(stderr, "current working directory get error: %s\n",\
        strerror(errno));
        return -1;
    }

    printf(" - [%4d] Current namespace Parent PID: %d\n",getpid(), getppid());
    printf("current working directory: %s\n", path);

    if(execl("/usr/local/bin/filebrowser", "filebrowser", "-p", "8282", \
    (char *)NULL)) {
        printf("exec failed\n");
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

    web_server_Pid = clone(child_func, stack + STACK_SIZE, CLONE_NEWUTS | CLONE_NEWIPC \
    | CLONE_NEWPID | CLONE_NEWNS | SIGCHLD, "Hello");
    if(web_server_Pid == -1) {
        errExit("clone\n");
    }

    printf("PID of child created by clone() is %ld\n", \
    (long)web_server_Pid);

    munmap(stack, STACK_SIZE);



    return 0;
}
