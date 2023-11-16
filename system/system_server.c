#include <stdio.h>
#include <sys/prctl.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

int system_server()
{
    printf("system_server Process...!\n");

    while (1) {
        sleep(1);
    }

    return 0;
}

int create_system_server()
{
    pid_t system_server_Pid;
    const char *processName = "system_server";

    printf("creating system_server...\n");

    switch (system_server_Pid = fork())
    {

    case -1:
        printf("system_server fork failed\n");
        break;
    case 0:
        if(prctl(PR_SET_NAME, (unsigned long)processName, NULL, NULL, NULL) == 0) {
            /* man prctl -> int prctl(int option, unsinged long arg2, unsigned long arg2, 
             unsigned long arg3, unsigned long arg4, unsigned long arg5)*/

            system_server();
            break;
        }
        
        perror("prctl()");
        break;
    
    default:
        break;
    }




    return 0;
}
