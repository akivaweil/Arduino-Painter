// PatternExecutor.cpp
#include "PatternExecutor.h"

#include <Arduino.h>

#include "Patterns.h"

PatternExecutor::PatternExecutor(MovementController& movement)
    : movementController(movement),
      currentSide(0),
      currentCommand(0),
      targetSide(-1),
      executingSingleSide(false)
{
}

void PatternExecutor::update()
{
    // Don't process if movement is still in progress
    if (movementController.isMoving())
    {
        return;
    }

    // Get current pattern information
    Command* pattern = getCurrentPattern();
    int patternSize = getCurrentPatternSize();

    // Process next command if available
    if (currentCommand < patternSize)
    {
        processNextCommand();
    }
    else
    {
        // Pattern complete for current side
        currentCommand = 0;

        if (executingSingleSide)
        {
            Serial.println(F("Single side pattern complete"));
            executingSingleSide = false;
            targetSide = -1;
        }
        else
        {
            // Move to next side for full pattern execution
            currentSide++;
            if (currentSide >= 4)
            {
                Serial.println(F("Full pattern complete"));
                currentSide = 0;
                executingSingleSide = false;
            }
            else
            {
                Serial.print(F("Moving to side "));
                Serial.println(currentSide);
            }
        }
    }
}

void PatternExecutor::startPattern()
{
    currentSide = 0;
    currentCommand = 0;
    executingSingleSide = false;
    targetSide = -1;
    Serial.println(F("Starting full pattern execution"));
}

void PatternExecutor::startSingleSide(int side)
{
    if (side >= 0 && side < 4)
    {
        currentSide = side;
        currentCommand = 0;
        executingSingleSide = true;
        targetSide = side;
        Serial.print(F("Starting pattern for side "));
        Serial.println(side);
    }
    else
    {
        Serial.println(F("ERROR: Invalid side selected"));
    }
}

bool PatternExecutor::isExecuting() const
{
    return currentCommand > 0 || currentSide > 0 || executingSingleSide;
}

Command* PatternExecutor::getCurrentPattern() const
{
    switch (currentSide)
    {
        case 0:
            return FRONT;
        case 1:
            return BACK;
        case 2:
            return LEFT;
        case 3:
            return RIGHT;
        default:
            return nullptr;
    }
}

int PatternExecutor::getCurrentPatternSize() const
{
    switch (currentSide)
    {
        case 0:
            return FRONT_SIZE;
        case 1:
            return BACK_SIZE;
        case 2:
            return LEFT_SIZE;
        case 3:
            return RIGHT_SIZE;
        default:
            return 0;
    }
}

void PatternExecutor::processNextCommand()
{
    Command* pattern = getCurrentPattern();
    if (pattern == nullptr)
    {
        Serial.println(F("ERROR: Invalid pattern"));
        return;
    }

    // Execute the next command in sequence
    Command currentCmd = pattern[currentCommand];
    if (movementController.executeCommand(currentCmd))
    {
        currentCommand++;

        // Log progress for debugging
        Serial.print(F("Executing command "));
        Serial.print(currentCommand);
        Serial.print(F(" of "));
        Serial.println(getCurrentPatternSize());
    }
    else
    {
        Serial.println(F("ERROR: Failed to execute pattern command"));
    }
}