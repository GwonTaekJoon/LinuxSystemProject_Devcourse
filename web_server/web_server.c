#include <stdio.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

int create_web_server()
{
    pid_t web_server_Pid;

    printf("creating web_server...\n");

    switch (web_server_Pid = fork())
    {
    case -1:
        printf("web_server fork failed\n");
        break;

    case 0:
        if (execl("/usr/local/bin/filebrowser", "filebrowser", "-p", "8284", (char *) NULL)) {
            /*int execl(char const *path, char const *arg0, ...);*/

            printf("web_server exec failed\n");
        }
        break;
    
    
    default:
        break;
    }



    return 0;
}
