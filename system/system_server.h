#ifndef _SYSTEM_SERVER_H
#define _SYSTEM_SERVER_H

#include <sys/types.h>  /* Type definitions used by many programs */
#include <stdio.h>      /* Standard I/O functions */
#include <stdlib.h>     /* Prototypes of commonly used library functions,
                           plus EXIT_SUCCESS and EXIT_FAILURE constants */
#include <unistd.h>     /* Prototypes for many system calls */
#include <errno.h>      /* Declares errno and defines error constants */
#include <string.h>     /* Commonly used string-handling functions */
#include <stdbool.h>    /* 'bool' type plus 'true' and 'false' constants */

int create_system_server();

#define USEC_PER_SECOND         1000000  /* one million */ 
#define USEC_PER_MILLISEC       1000     /* one thousand */
#define NANOSEC_PER_SECOND      1000000000 /* one BILLLION */
#define NANOSEC_PER_USEC        1000     /* one thousand */
#define NANOSEC_PER_MILLISEC    1000000  /* one million */
#define MILLISEC_PER_TICK       10
#define MILLISEC_PER_SECOND     1000

#endif /* _SYSTEM_SERVER_H */
