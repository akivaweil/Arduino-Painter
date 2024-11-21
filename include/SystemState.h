// SystemState.h
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

enum SystemState
{
    IDLE,
    HOMING_X,
    HOMING_Y,
    HOMED,
    EXECUTING_PATTERN,
    ERROR,
    CYCLE_COMPLETE,
    PRIMING,
    CLEANING,
    CALIBRATING,
    PAINTING_SIDE,
    STOPPED,
    PAUSED,
};

#endif