#include <stdio.h>
#include <sys/wait.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

int main()
{
    pid_t spid;
    pid_t gpid;
    pid_t ipid;
    pid_t wpid;

    int status;
    int savedErrno;

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
