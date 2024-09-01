#include "stdafx.h"
#include "LeftMotor.h"
#include "RightMotor.h"

#ifdef __cplusplus
extern "C" {
#endif

using namespace std;

static RightMotor rmotor;
static LeftMotor lmotor;

int set_right_motor_speed(int speed)
{
    MotorData *data = new MotorData();
    data -> speed = speed;
    rmotor.SetSpeed(data);

    return 0;
}

int halt_right_motor(void)
{
    rmotor.Halt();
    return 0;
}

int set_left_motor_speed(int speed)
{
    MotorData *data = new MotorData();
    data -> speed = speed;
    lmotor.SetSpeed(data);

    return 0;
}

int halt_left_motor(void)
{
    lmotor.Halt();
    return 0;
}

#ifdef __cplusplus

}
#endif