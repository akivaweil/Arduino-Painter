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
    CALIBRATING,
    PAINTING_SIDE,
    STOPPED,
    PAUSED,
    MANUAL_ROTATING,  // New state for manual rotation
    DEPRESSURIZE_POT,
};

#endif