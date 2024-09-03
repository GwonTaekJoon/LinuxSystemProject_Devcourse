#ifndef _TOY_MESSAGE_H_
#define _TOY_MESSAGE_H_


#include <unistd.h>

typedef struct {
    unsigned int msg_type;
    unsigned int param1;
    unsigned int param2;
    void* param3;

} toy_msg_t;

#define MESSAGE_DATA_MAX_LEN 50
#define MESSAGE_HOSTNAME_MAX_LEN 32

enum ROBOT_MESSAGE_ID {
    MESSAGE_ID_EXIT = 0,
    MESSAGE_ID_INFO,
    MESSAGE_ID_MAX
};

typedef struct robot_info {
    unsigned int id;
    char hostname[MESSAGE_HOSTNAME_MAX_LEN];
    unsigned int temperature;
} robot_info_t; //40byte

union robot_message_data {
    robot_info_t info;
    char data[MESSAGE_DATA_MAX_LEN];
};

typedef struct robot_message {
    unsigned int id;
    union robot_message_data data;
} robot_message_t;

#define CMD_ROBOT_INFO 0x0011
#endif
