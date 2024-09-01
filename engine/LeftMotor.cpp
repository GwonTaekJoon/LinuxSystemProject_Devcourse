#include "LeftMotor.h"
#include <iostream>
#include <stdlib.h>
#include <sys/stat.h>
#include <unistd.h>
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

LeftMotor::LeftMotor() :
    StateMachine(ST_MAX_STATES),
    m_currentSpeed(0)
{
}

void LeftMotor::StartMotorSpeed(int speed)
{
    cout << "Call toy_engine_driver: " << speed << endl;
    int dev = open("/dev/toy_engine_driver", O_RDWR | O_NDELAY);
    if(dev < 0) {
        cout << "module open error" << endl;
        return;
    }

    if(ioctl(dev, MOTOR_1_START_SPEED, &speed) < 0) {
        cout << "Error while ioctl READ_USE_COUNT (before) in main\n" << endl;
    }

    close(dev);
}

void LeftMotor::SetMotorSpeed(int speed)
{
    cout << "Call toy_engine_driver: " << speed << endl;
    int dev = open("/dev/toy_engine/driver", O_RDWR | O_NDELAY);
    if(dev < 0) {
        cout << "module open error" << endl;
        return;
    }

    if(ioctl(dev, MOTOR_1_SET_SPEED, &speed) < 0) {
        cout << "Error while ioctl READ_USE_COUNT (before) in main\n" << endl;
    }

    close(dev);
}

void LeftMotor::HaltMotor()
{
    int halt;

    cout << "Call toy_engine_driver : " << endl;
    int dev = open("/dev/toy_engine_driver", O_RDWR | O_NDELAY);
    if(dev < 0) {
        cout << "module open error" << endl;
        return;
    }

    if(ioctl(dev, MOTOR_1_HALT, &halt) < 0) {
        cout << "Error while ioctl READ_USE_COUNT (before) in main\n" << endl;
    }

    close(dev);
}

// set motor external event
void LeftMotor::SetSpeed(MotorData *data)
{
    BEGIN_TRANSITION_MAP    // - Current State -
        TRANSITION_MAP_ENTRY(ST_START) // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN) //ST_STOP
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED) //ST_START
        TRANSITION_MAP_ENTRY(ST_CHANGE_SPEED) // ST_CHANGE_SPEED
    END_TRANSITION_MAP(data)

}

void LeftMotor::Halt()
{
    BEGIN_TRANSITION_MAP // - Current State - 
        TRANSITION_MAP_ENTRY(EVENT_IGNORED) // ST_IDLE
        TRANSITION_MAP_ENTRY(CANNOT_HAPPEN) // ST_STOP
        TRANSITION_MAP_ENTRY(ST_STOP) // ST_START
        TRANSITION_MAP_ENTRY(ST_STOP) // ST_CHANGE_SPEED
    END_TRANSITION_MAP(NULL)
}

STATE_DEFINE(LeftMotor, Idle, NoEventData)
{
    cout << "LeftMotor::ST_IDLE" << endl;
    m_currentSpeed = 0;

    // perform the stop motor porcessing here
    // transition to Idle via an internal event
    InternalEvent(ST_IDLE);
    HaltMotor();
}

// stop the motor
STATE_DEFINE(LeftMotor, Stop, NoEventData)
{
	cout << "LeftMotor::ST_Stop" << endl;
	m_currentSpeed = 0;

	// perform the stop motor processing here
	// transition to Idle via an internal event
	InternalEvent(ST_IDLE);
	HaltMotor();
}

// start the motor going
STATE_DEFINE(LeftMotor, Start, MotorData)
{
    cout << "LeftMotor::St_Start: Speed is " << data -> speed << endl;
    m_currentSpeed = data -> speed;

    // set initial motor speed processing here
    StartMotorSpeed(data -> speed);
}

// changes the motor speed once the motor is moving
STATE_DEFINE(LeftMotor, ChangeSpeed, MotorData)
{
    cout << "LeftMotor::ST_ChangeSpeed : Speed is" << data -> speed << endl;
    m_currentSpeed = data -> speed;

    //perform the change motor speed to data -> speed here
    SetMotorSpeed(data -> speed);
}

