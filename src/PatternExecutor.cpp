// PatternExecutor.cpp
#include "PatternExecutor.h"

#include <Arduino.h>

#include "Patterns.h"

PatternExecutor::PatternExecutor(MovementController& movement)
    : movementController(movement),
      currentSide(-1),
      currentCommand(-1),
      targetSide(-1),
      executingSingleSide(false),
      stopped(false)  // Initialize new member
{
}

void PatternExecutor::update()
{
    // Don't process if stopped or movement is still in progress
    if (stopped || movementController.isMoving())
    {
        return;
    }

    // Get current pattern information
    Command* pattern = getCurrentPattern();
    int patternSize = getCurrentPatternSize();

    // Add debug logging
    Serial.print(F("Pattern size: "));
    Serial.println(patternSize);

    // Process next command if available
    if (currentCommand < patternSize)
    {
        Serial.println(F("Processing next command..."));
        processNextCommand();
    }
    else
    {
        // Pattern complete for current side
        Serial.println(F("Pattern complete for current side"));
        currentCommand = 0;

        if (executingSingleSide)
        {
            Serial.println(F("Single side pattern complete"));
            executingSingleSide = false;
            targetSide = -1;
            stateManager->setState(
                HOMED);  // Return to HOMED state after single side
        }
        else
        {
            // Move to next side for full pattern execution
            currentSide++;
            if (currentSide >= 4)
            {
                Serial.println(F("Full pattern complete"));
                // Reset pattern execution state
                currentSide = -1;
                currentCommand = -1;
                executingSingleSide = false;
                stateManager->setState(
                    CYCLE_COMPLETE);  // Transition to IDLE after full pattern
                // Reset speeds to default after pattern completion
                movementController.resetToDefaultSpeed();
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
    stopped = false;  // Clear stopped flag when starting new pattern
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
        stopped = false;  // Clear stopped flag when starting new pattern
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
    return currentCommand >= 0 || currentSide >= 0 || executingSingleSide;
}

Command* PatternExecutor::getCurrentPattern() const
{
    Serial.print(F("Getting pattern for side: "));
    Serial.println(currentSide);

    switch (currentSide)
    {
        case 0:
            Serial.println(F("Returning FRONT pattern"));
            return FRONT;
        case 1:
            Serial.println(F("Returning BACK pattern"));
            return BACK;
        case 2:
            Serial.println(F("Returning LEFT pattern"));
            return LEFT;
        case 3:
            Serial.println(F("Returning RIGHT pattern"));
            return RIGHT;
        default:
            Serial.println(F("ERROR: No pattern available"));
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

    // Apply pattern-specific speed before executing command
    movementController.applyPatternSpeed(getCurrentPatternName());

    // Execute the next command in sequence
    Command currentCmd = pattern[currentCommand];
    if (movementController.executeCommand(currentCmd))
    {
        currentCommand++;

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

void PatternExecutor::stop()
{
    // Reset all execution state
    currentSide = -1;
    currentCommand = -1;
    targetSide = -1;
    executingSingleSide = false;
    stopped = true;
    Serial.println(F("Pattern execution stopped"));
}

String PatternExecutor::getCurrentPatternName() const
{
    switch (currentSide)
    {
        case 0:
            return "FRONT";
        case 1:
            return "BACK";
        case 2:
            return "LEFT";
        case 3:
            return "RIGHT";
        default:
            return "";
    }
}