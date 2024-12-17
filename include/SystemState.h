// Update SystemState.h
#ifndef SYSTEM_STATE_H
#define SYSTEM_STATE_H

enum SystemState
{
    IDLE,
    HOMING_X,
    HOMING_Y,
    HOMING_ROTATION,
    HOMED,
    EXECUTING_PATTERN,
    ERROR,
    CYCLE_COMPLETE,
    PRIMING,
    CLEANING,
    PAINTING_SIDE,
    STOPPED,
    PAUSED,
    MANUAL_ROTATING,
    BACK_WASHING,
};

#endif