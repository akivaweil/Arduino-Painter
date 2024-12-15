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
            return true;

        case ERROR:
            return (newState == IDLE || newState == HOMING_X ||
                    newState == STOPPED);

        case HOMING_X:
            return (newState == ERROR || newState == HOMING_Y ||
                    newState == STOPPED || newState == HOMED);

        case HOMING_Y:
            return (newState == ERROR || newState == HOMING_ROTATION ||
                    newState == STOPPED || newState == HOMED);

        case HOMING_ROTATION:
            return (newState == ERROR || newState == HOMED ||
                    newState == STOPPED);

        case HOMED:
            return (newState != ERROR && newState != IDLE) ||
                   newState == MANUAL_ROTATING;

        case EXECUTING_PATTERN:
            return (newState == ERROR || newState == IDLE ||
                    newState == STOPPED || newState == HOMING_X);

        case STOPPED:
            return (newState == HOMING_X);

        case MANUAL_ROTATING:  // New case for manual rotation
            return (newState == ERROR || newState == IDLE ||
                    newState == HOMED || newState == STOPPED);

        case PRIMING:
        case CLEANING:
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
            case PRIMING:
                Serial.println(F("PRIMING"));
                break;
            case CLEANING:
                Serial.println(F("CLEANING"));
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
            case MANUAL_ROTATING:
                Serial.println(F("MANUAL_ROTATING"));
                break;
        }
    }
}