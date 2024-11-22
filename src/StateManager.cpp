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
            return (newState != ERROR && newState != CYCLE_COMPLETE);

        case ERROR:
            return (newState == IDLE || newState == HOMING_X ||
                    newState == STOPPED);

        case HOMING_X:
            return (newState == ERROR || newState == HOMING_Y ||
                    newState == STOPPED);

        case HOMING_Y:
            return (newState == ERROR || newState == HOMING_ROTATION ||
                    newState == STOPPED);

        case HOMING_ROTATION:
            return (newState == ERROR || newState == HOMED ||
                    newState == STOPPED);

        case HOMED:
            return (newState != ERROR || newState != CYCLE_COMPLETE ||
                    newState != IDLE || newState == STOPPED);

        case EXECUTING_PATTERN:
            return (newState == ERROR || newState == CYCLE_COMPLETE ||
                    newState == HOMING_X || newState == STOPPED);

        case CYCLE_COMPLETE:
            return (newState == IDLE || newState == HOMING_X ||
                    newState == STOPPED);

        case STOPPED:
            return (newState == HOMING_X);

        case PRIMING:
        case CLEANING:
        case CALIBRATING:
        case PAINTING_SIDE:
            return (newState == ERROR || newState == IDLE ||
                    newState == HOMING_X || newState == STOPPED);

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
            case HOMED:
                Serial.println(F("HOMED"));
                break;
            case PAUSED:
                Serial.println(F("PAUSED"));
                break;
            case STOPPED:
                Serial.println(F("STOPPED"));
                break;
            case HOMING_ROTATION:
                Serial.println(F("HOMING_ROTATION"));
                break;
        }
    }
}