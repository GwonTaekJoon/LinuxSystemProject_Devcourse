#include <stdio.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

int create_gui()
{
    pid_t gui_Pid;

    printf("creating gui_server...\n");

    sleep(5);

    switch (gui_Pid = fork())
    {
    case -1:
        printf("gui fork failed\n");
        break;
    
    case 0:
        if (execl("/usr/bin/google-chrome-stable", "google-chrome-stable", "http://localhost:8284", NULL)) {
            /*int execl(char const *path, char const *arg0, ...);*/
            printf("gui exec failed\n");
        }

        break;


    default:
        break;
    }



    return 0;
}
