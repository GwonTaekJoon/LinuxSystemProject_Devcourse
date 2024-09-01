#ifndef _RIGHTMOTOR_H
#define _RIGHTMOTOR_H

#include "Motor.h"

class RightMotor : public StateMachine
{
public:
    RightMotor();

    //External events taken by this state machine
    void SetSpeed(MotorData *data);
    void Halt();

private:
    INT m_currentSpeed;

    // State enumeration order must match the order of state method entries
    // in the state map.
    enum States
    {
        ST_IDLE,
        ST_STOP,
        ST_START,
        ST_CHANGE_SPEED,
        ST_MAX_STATES
    };

    //Define the state machine functions with event data type
    STATE_DECLARE(RightMotor, Idle, NoEventData)
    STATE_DECLARE(RightMotor, Stop, NoEventData)
    STATE_DECLARE(RightMotor, Start, MotorData)
    STATE_DECLARE(RightMotor, ChangeSpeed, MotorData)

    // State map to define state object order. Each state map entry defines a 
    // state object.
    BEGIN_STATE_MAP
        STATE_MAP_ENTRY(&Idle)
        STATE_MAP_ENTRY(&Stop)
        STATE_MAP_ENTRY(&Start)
        STATE_MAP_ENTRY(&ChangeSpeed)
    END_STATE_MAP

    void StartMotorSpeed(int speed);
    void SetMotorSpeed(int speed);
    void HaltMotor();

};
#endif