// StateManager.cpp
#include "StateManager.h"

#include <Arduino.h>

StateManager::StateManager() : currentState(IDLE), previousState(IDLE) {}

SystemState StateManager::getCurrentState() const { return currentState; }

void StateManager::setState(SystemState newState)
{
    if (isValidTransition(newState))
    {
        previousState = currentState;
        currentState = newState;
        reportStateChange();
    }
}

bool StateManager::isValidTransition(SystemState newState) const
{
    switch (currentState)
    {
        case IDLE:
            // From IDLE, we can transition to any state except ERROR and
            // CYCLE_COMPLETE
            return (newState != ERROR && newState != CYCLE_COMPLETE);

        case ERROR:
            // From ERROR, we can only transition back to IDLE
            return (newState == IDLE);

        case HOMING_X:
            // From HOMING_X, we can only go to ERROR or HOMING_Y
            return (newState == ERROR || newState == HOMING_Y);

        case HOMING_Y:
            // From HOMING_Y, we can go to ERROR, IDLE, or start
            // EXECUTING_PATTERN
            return (newState == ERROR || newState == IDLE ||
                    newState == EXECUTING_PATTERN);

        case EXECUTING_PATTERN:
            // From EXECUTING_PATTERN, we can go to ERROR or CYCLE_COMPLETE
            return (newState == ERROR || newState == CYCLE_COMPLETE);

        case CYCLE_COMPLETE:
            // From CYCLE_COMPLETE, we can only go back to IDLE
            return (newState == IDLE);

        case PRIMING:
        case CLEANING:
        case CALIBRATING:
        case PAINTING_SIDE:
            // From maintenance states, we can go to ERROR or back to IDLE
            return (newState == ERROR || newState == IDLE);

        default:
            return false;
    }
}

void StateManager::reportStateChange()
{
    if (previousState != currentState)
    {
        Serial.print(F("State changed: "));

        switch (currentState)
        {
            case IDLE:
                Serial.println(F("IDLE"));
                break;
            case HOMING_X:
                Serial.println(F("HOMING_X"));
                break;
            case HOMING_Y:
                Serial.println(F("HOMING_Y"));
                break;
            case EXECUTING_PATTERN:
                Serial.println(F("EXECUTING_PATTERN"));
                break;
            case ERROR:
                Serial.println(F("ERROR"));
                break;
            case CYCLE_COMPLETE:
                Serial.println(F("CYCLE_COMPLETE"));
                break;
            case PRIMING:
                Serial.println(F("PRIMING"));
                break;
            case CLEANING:
                Serial.println(F("CLEANING"));
                break;
            case CALIBRATING:
                Serial.println(F("CALIBRATING"));
                break;
            case PAINTING_SIDE:
                Serial.println(F("PAINTING_SIDE"));
                break;
        }
    }
}