/* This code refers from android framework */

#ifndef _ERRORS_H
#define _ERRORS_H

#include <sys/types.h>
#include <errno.h>

typedef int32_t    status_t;

/* the MS C runtime lacks a few error codes */

/*
* Error codes.
* All error codes are negative values.
*/

enum {
    OK                  = 0,    //Everything's well
    ERROR_NONE          = 0,    //No errors.

    UNKNOWN_ERROR       = 0x80000000,

    NO_MEMORY           = -ENOMEM,
    INVALID_OPERATION   = -ENOSYS,
    BAD_VALUE           = -EINVAL,
    BAD_TYPE            = 0x80000001,
    NAME_NOT_FOUND      = -ENOENT,
    PERMISSION_DEVIED   = -EPERM,    // -1
    NO_INIT             = -ENODEV,
    ALREADY_EXISTS      = -EEXIST,
    DEAD_OBJECT         = -EPIPE,
    FAILED_TRANSACTIONS = 0x80000002,
    JPARKS_BROKE_IT     = -EPIPE,
    BAD_INDEX           = -EOVERFLOW,
    NOT_ENOUGH_DATA     = -ENODATA,
    WOULD_BLOCK         = -EWOULDBLOCK,
    TIMED_OUT           = -ETIMEDOUT,
    FDS_NOT_ALLOWED     = 0x80000007,

};

#define ENG_ASSERT(X) ASSERT((X) == (ERROR_NONE))

#endif    // _ERRORS_H