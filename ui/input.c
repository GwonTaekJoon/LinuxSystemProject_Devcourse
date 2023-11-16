#include <stdio.h>
#include <sys/prctl.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

int input()
{
    printf("input()...\n");

    while (1) {
        sleep(1);
    }

    return 0;
}

int create_input()
{
    pid_t inputPid;
    const char *processName = "input";

    printf("creating input...\n");

    switch (inputPid = fork())
    {
    case -1:
        printf("input fork failed\n");
        break;
    
    case 0:
        if(prctl(PR_SET_NAME, (unsigned long)processName, NULL, NULL, NULL) == 0){
            /* man prctl -> int prctl(int option, unsinged long arg2, unsigned long arg2, 
             unsigned long arg3, unsigned long arg4, unsigned long arg5)*/
            input();
            break;

        }
        perror("prctl()");
    default:
        break;
    }

    return 0;
}
