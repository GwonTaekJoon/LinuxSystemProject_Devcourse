#ifndef _MOTOR_H
#define _MOTOR_H

#include "StateMachine.h"

class MotorData : public EventData
{
public:
    INT speed;
};

#endif