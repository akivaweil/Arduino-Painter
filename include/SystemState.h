// SystemState.h
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

enum SystemState
{
    IDLE,
    HOMING_X,
    HOMING_Y,
    EXECUTING_PATTERN,
    ERROR,
    CYCLE_COMPLETE
};

#endif