#include <stdio.h>
#include <sys/prctl.h>
#include <signal.h>
#include <sys/time.h>
#include <time.h>

#include <system_server.h>
#include <gui.h>
#include <input.h>
#include <web_server.h>

static int toy_timer = 0;


void sighandler_timer() {
    ++toy_timer;
    time_t rawTime;
    struct tm* formattedTime;
    rawTime = time(NULL);
    /*rawTime has a time value in seconds since January 1, 1970 */

    formattedTime = localtime(&rawTime);
    /*localtime function turns rawTime into easily recognizable value using tm sturct*/

    int year = formattedTime -> tm_year + 1900;
    int month = formattedTime -> tm_mon + 1;
    int day = formattedTime -> tm_mday;
    int hour = formattedTime -> tm_hour;
    int min = formattedTime -> tm_min;
    int sec = formattedTime -> tm_sec;

    printf("Current Time Info : %d year %d month %d day %d:%d:%d\ntimer_count : %d\n"
    ,year, month, day, hour, min, sec, toy_timer);



}
int posix_sleep_ms(unsigned int timeout_ms)
{
    struct timespec sleep_time;

    sleep_time.tv_sec = timeout_ms / MILLISEC_PER_SECOND;
    sleep_time.tv_nsec = (timeout_ms % MILLISEC_PER_SECOND) * (NANOSEC_PER_USEC * USEC_PER_MILLISEC);

    return nanosleep(&sleep_time, NULL);
}



int system_server()
{

    struct itimerspec ts;
    struct sigaction  sa;
    struct sigevent   sev;
    timer_t *tidlist;
    printf("system_server Process...!\n");

    memset(&sa, 0, sizeof(sigaction));
    sigemptyset(&sa.sa_mask);
    //safe init

    sa.sa_flags = 0;
    sa.sa_handler = sighandler_timer;

    ts.it_value.tv_sec = 5;
    ts.it_value.tv_nsec = 0;
    ts.it_interval.tv_sec = 5;
    ts.it_interval.tv_nsec = 0;

    if(sigaction(SIGALRM, &sa, NULL) == -1) {

        fprintf(stderr, "timer signal init error");


    }

    if(setitimer(ITIMER_REAL, &ts, NULL) == -1) {

        fprintf(stderr,"set timer error");
        exit(-1);

    }


    while (1) {
        posix_sleep_ms(5000);
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
