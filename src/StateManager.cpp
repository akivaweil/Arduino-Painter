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
    return true;  // Allow all state transitions
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