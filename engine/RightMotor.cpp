#include "RightMotor.h"
#include <iostream>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <sys/types.h>
#include <sys/stat.h>

#define MOTOR_1_START_SPEED _IOW('w', '1', int32_t *)
#define MOTOR_2_START_SPEED _IOW('w', '2', int32_t *)
#define MOTOR_1_SET_SPEED _IOW('w', '3', int32_t *)
#define MOTOR_2_SET_SPEED _IOW('w', '4', int32_t *)
#define MOTOR_1_HALT _IOW('w', '5', int32_t *)
#define MOTOR_2_HALT _IOW('w', '6', int32_t *)

using namespace std;

RightMotor::RightMotor() :
    StateMachine(ST_MAX_STATES),
    m_currentSpeed(0)
{
}

void RightMotor::StartMotorSpeed(int speed)
{
    cout << "Call toy_engine_driver : " << speed << endl;
    int dev = open("/dev/toy_engine_driver", O_RDWR | O_NDELAY);
    if(dev < 0) {
        cout << "module open error" << endl;
        return;
    }

    if(ioctl(dev, MOTOR_2_START_SPEED, &speed) < 0) {
        cout << "Error while ioctl READ_USE_COUNT (before) in main\n" << endl;
    }

    close(dev);
}

void RightMotor::SetMotorSpeed(int speed)
{
    cout << "cout toy_engine_driver : " << speed << endl;
    int dev = open("/dev/toy_engine_driver", O_RDWR | O_NDELAY);
    if(dev < 0) {
        cout << "module open error" << endl;
        return;
    }

    if(ioctl(dev, MOTOR_2_SET_SPEED, &speed) < 0) {
        cout << "Error while ioctl READ_USE_COUNT (befor) in main\n" << endl;
    }
    close(dev);
}

void RightMotor::HaltMotor()
{
    int halt;

    cout << "Call toy_engine_driver: " << endl;
    int dev = open("/dev/toy_engine_driver", O_RDWR | O_NDELAY);
    if(dev < 0) {
        cout << "module open error" << endl;
        return;
    }

    if(ioctl(dev, MOTOR_2_HALT, &halt) < 0) {
        cout << "Error while ioctl READ_USE_COUNT (before) in main\n" << endl;
    }
    close(dev);
}

// set motor speed external event
void RightMotor::SetSpeed(MotorData *data)
{
    BEGIN_TRANSITION_MAP // - Current State -
        TRANSITION_MAP_ENTRY(ST_START) // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN) // ST_STOP
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED) //ST_START
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED) // ST_CHANGE_SPEED
    END_TRANSITION_MAP(data)
}

// halt motor external event
void RightMotor::Halt()
{
    BEGIN_TRANSITION_MAP // - Current State
        TRANSITION_MAP_ENTRY(EVENT_IGNORED) // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN) // ST_STOP
        TRANSITION_MAP_ENTRY(ST_STOP) // ST_START
        TRANSITION_MAP_ENTRY(ST_STOP) // ST_CHANGE_SPEED
    END_TRANSITION_MAP(NULL)
}

// state machine sits here when motor is not running
STATE_DEFINE(RightMotor, Idle, NoEventData)
{
    cout << "RightMotor::ST_Idle" << endl;
}

STATE_DEFINE(RightMotor, Stop, NoEventData)
{
    cout << "RightMotor::ST_Stop" << endl;
    m_currentSpeed = 0;

    // perform the stop motor processing here
    // transition to Idle via an internal event
    InternalEvent(ST_IDLE);
    HaltMotor();
}

// start the motor going
STATE_DEFINE(RightMotor, Start, MotorData)
{
    cout << "RightMotor::ST_Start : Speed is " << data -> speed << endl;
    m_currentSpeed = data -> speed;

    //set initial motor speed processing here
    StartMotorSpeed(data -> speed);
}

STATE_DEFINE(RightMotor, ChangeSpeed, MotorData)
{
    cout << "RightMotor::ST_ChangeSpeed : Speed is " << data -> speed << endl;
    m_currentSpeed = data -> speed;

    // perform the change motor speed to data -> speed here
    SetMotorSpeed(data -> speed);
}

