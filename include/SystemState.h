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
    PRIMING,       // New state for gun priming
    CLEANING,      // New state for gun cleaning
    CALIBRATING,   // New state for quick calibration
    PAINTING_SIDE  // New state for single side painting
};

#endif