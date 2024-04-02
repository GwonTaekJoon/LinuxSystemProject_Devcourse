#ifndef _TOY_MESSAGE_H_
#define _TOY_MESSAGE_H_


#include <unistd.h>

typedef struct {
    unsigned int msg_type;
    unsigned int param1;
    unsigned int param2;
    void* param3;

} toy_msg_t;

#endif
