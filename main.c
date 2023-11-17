#include <stdio.h>
#include <sys/wait.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>
#include <string.h>

/* 여기서 sigchld 핸들러 구현 */
static void
sigchldHandler(int sig)
{   
    int status;
     int tmp_errno = errno;
     pid_t childPid;

    /*errno is a global variable provided by the standard library of language C,
     indicating the cause of errors when system calls and library functions fail.*/

    while ((childPid = waitpid(-1, &status, WNOHANG)) > 0) {
        printf("childPid = %d terminated\n", childPid);
    }

    /*All child processes are terminated */
    
    errno = tmp_errno;
    //restored errno
}

int main()
{
    pid_t spid;
    pid_t gpid;
    pid_t ipid;
    pid_t wpid;

    int status;

    struct sigaction sa;

    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    sa.sa_handler = sigchldHandler;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        fprintf(stderr,"signal init error");
        return 1;
    }

    printf("main()...\n");

    printf("create_system server()...\n");
    spid = create_system_server();

    
    printf("create_web_server()...\n");
    wpid = create_web_server();

    
    printf("create_input()...\n");
    ipid = create_input();

    printf("create_gui()...\n");
    gpid = create_gui();

    


    waitpid(spid, &status, 0);
    waitpid(gpid, &status, 0);
    waitpid(ipid, &status, 0);
    waitpid(wpid, &status, 0);




    return 0;
}
