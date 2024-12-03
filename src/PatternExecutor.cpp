#include "PatternExecutor.h"

#include <Arduino.h>
#include <config.h>

#include "Patterns.h"

// Structured status reporting
void PatternExecutor::reportStatus(const char* event, const String& details)
{
    String status = String(event) + "|";
    status += "side=" + String(currentSide) + "|";
    status += "pattern=" + getCurrentPatternName() + "|";
    status += "row=" + String(currentRow + 1) + "|";  // 1-based for display
    status += "command=" + String(currentCommand) + "|";
    status += "total_commands=" + String(getCurrentPatternSize()) + "|";
    status += "single_side=" + String(executingSingleSide ? "true" : "false");

    // Add movement-specific information for MOVE_X and MOVE_Y events
    Command* pattern = getCurrentPattern();
    if (pattern != nullptr && currentCommand >= 0 &&
        currentCommand < getCurrentPatternSize())
    {
        Command currentCmd = pattern[currentCommand];
        if ((strcmp(event, "MOVE_X") == 0 &&
             (currentCmd.type == 'X' || currentCmd.type == 'M')) ||
            (strcmp(event, "MOVE_Y") == 0 &&
             (currentCmd.type == 'Y' || currentCmd.type == 'N')))
        {
            float duration = calculateMovementDuration(currentCmd);
            status += "|distance=" + String(abs(currentCmd.value), 3);
            status += "|direction=" +
                      String(currentCmd.value < 0 ? "negative" : "positive");
            status += "|spray=" + String(currentCmd.sprayOn ? "on" : "off");
            status += "|duration_ms=" + String(duration, 2);
            // Add movement direction indicator
            status +=
                "|movement_axis=" +
                String(currentCmd.type == 'X' || currentCmd.type == 'M' ? "X"
                                                                        : "Y");
        }
    }

    // Add row transition details for Y movements
    if (strcmp(event, "MOVE_Y") == 0 && pattern != nullptr)
    {
        Command currentCmd = pattern[currentCommand];
        if (currentCmd.type == 'Y' || currentCmd.type == 'N')
        {
            int targetRow =
                currentRow +
                (currentCmd.value > 0 ? 2 : 0);  // +2 for positive moves (next
                                                 // row), same for negative
            status += "|details=to_row_" + String(targetRow);
        }
    }

    if (details.length() > 0)
    {
        status += "|details=" + details;
    }

    Serial.println(status);
}

// Rest of the file remains unchanged
float PatternExecutor::calculateMovementDuration(const Command& cmd) const
{
    // Get current speeds and steps per inch from MovementController
    float distance = abs(cmd.value);  // Distance in inches
    float steps;
    float speed;

    // Calculate steps and get appropriate speed based on command type
    if (cmd.type == 'X' || cmd.type == 'M')
    {
        steps = distance * X_STEPS_PER_INCH;
        // Use pattern-specific speed if available
        speed = movementController.getCurrentXSpeed();
    }
    else if (cmd.type == 'Y' || cmd.type == 'N')
    {
        steps = distance * Y_STEPS_PER_INCH;
        speed = Y_SPEED;  // Y movements always use default speed
    }
    else
    {
        return 0.0;  // Not a movement command
    }

    // Calculate duration in milliseconds
    // Duration = (steps / steps_per_second) * 1000
    return (steps / speed) * 1000.0;
}

PatternExecutor::PatternExecutor(MovementController& movement,
                                 HomingController& homing)
    : movementController(movement),
      homingController(homing),
      stateManager(nullptr),
      currentSide(-1),
      currentCommand(-1),
      targetSide(-1),
      executingSingleSide(false),
      stopped(false),
      currentRow(0)
{
}

void PatternExecutor::update()
{
    if (stopped || movementController.isMoving())
    {
        return;
    }

    Command* pattern = getCurrentPattern();
    int patternSize = getCurrentPatternSize();

    if (currentCommand < patternSize)
    {
        processNextCommand();
    }
    else
    {
        reportStatus("SIDE_COMPLETE", "");
        currentCommand = 0;
        currentRow = 0;

        if (executingSingleSide)
        {
            reportStatus("PATTERN_COMPLETE", "single_side");
            executingSingleSide = false;
            targetSide = -1;
            stateManager->setState(IDLE);
        }
        else
        {
            currentSide++;
            if (currentSide >= 4)
            {
                reportStatus("PATTERN_COMPLETE", "all_sides");
                currentSide = -1;
                currentCommand = -1;
                executingSingleSide = false;
                movementController.resetToDefaultSpeed();

                // Return rotation to home position
                long homePos = homingController.getHomeRotationPosition();
                Command rotateHome('R', 0, false);  // Rotate to 0 degrees
                movementController.executeCommand(rotateHome);

                // Wait for rotation to complete before homing X and Y
                if (!movementController.isMoving())
                {
                    homingController.startHoming();
                    reportStatus("AUTO_HOMING", "starting_after_pattern");
                }
            }
            else
            {
                // Calculate rotation needed for next side
                int rotationNeeded = 0;
                switch (currentSide)
                {
                    case 1:  // Going to BACK
                        rotationNeeded = 180;
                        break;
                    case 2:  // Going to LEFT
                        rotationNeeded = 90;
                        break;
                    case 3:  // Going to RIGHT
                        rotationNeeded = 180;
                        break;
                }

                if (rotationNeeded != 0)
                {
                    Command rotateCmd('R', rotationNeeded, false);
                    movementController.executeCommand(rotateCmd);
                    currentRotation = (currentRotation + rotationNeeded) % 360;
                }

                reportStatus("SIDE_CHANGE",
                             "moving_to_side_" + String(currentSide));
            }
        }
    }
}

void PatternExecutor::startPattern()
{
    stopped = false;
    currentSide = 0;
    currentCommand = 0;
    currentRow = 0;
    currentRotation = 0;  // Reset rotation tracking
    executingSingleSide = false;
    targetSide = -1;
    reportStatus("PATTERN_START", "full_pattern");
}

void PatternExecutor::startSingleSide(int side)
{
    if (side >= 0 && side < 4)
    {
        stopped = false;
        currentSide = side;
        currentCommand = 0;
        currentRow = 0;
        executingSingleSide = true;
        targetSide = side;

        // Calculate required rotation for the requested side
        int targetRotation;
        switch (side)
        {
            case 0:  // FRONT
                targetRotation = 0;
                break;
            case 1:  // BACK
                targetRotation = 180;
                break;
            case 2:  // LEFT
                targetRotation = 90;
                break;
            case 3:  // RIGHT
                targetRotation = 270;
                break;
        }

        // Add rotation command to get to correct position
        int rotationNeeded = targetRotation - currentRotation;
        if (rotationNeeded != 0)
        {
            // Normalize to shortest path (-180 to +180)
            if (rotationNeeded > 180)
                rotationNeeded -= 360;
            if (rotationNeeded < -180)
                rotationNeeded += 360;

            Command rotateCmd('R', rotationNeeded, false);
            movementController.executeCommand(rotateCmd);
        }

        currentRotation = targetRotation;  // Update current rotation
        reportStatus("PATTERN_START", "single_side");
    }
    else
    {
        reportStatus("ERROR", "invalid_side_selected");
    }
}

bool PatternExecutor::isExecuting() const
{
    return currentCommand >= 0 || currentSide >= 0 || executingSingleSide;
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
        reportStatus("ERROR", "invalid_pattern");
        return;
    }

    movementController.applyPatternSpeed(getCurrentPatternName());
    Command currentCmd = pattern[currentCommand];

    // Report movement events before execution
    if (currentCmd.type == 'X' || currentCmd.type == 'M')
    {
        reportStatus("MOVE_X", "");
    }
    else if (currentCmd.type == 'Y' || currentCmd.type == 'N')
    {
        reportStatus("MOVE_Y", "");
    }

    if (currentCommand > 0)
    {
        Command prevCmd = pattern[currentCommand - 1];

        // Track spray pattern progress
        if (currentCmd == SPRAY_OFF())
        {
            reportStatus("SPRAY_COMPLETE", "row_" + String(currentRow + 1));
        }
        else if (currentCmd == SPRAY_ON() && (prevCmd == MOVE_Y(4.16, false) ||
                                              prevCmd == MOVE_Y(-4.16, false) ||
                                              prevCmd == MOVE_Y(4.415, false) ||
                                              prevCmd == MOVE_Y(-4.415, false)))
        {
            currentRow++;
            reportStatus("SPRAY_START", "row_" + String(currentRow + 1));
        }
    }

    if (movementController.executeCommand(currentCmd))
    {
        currentCommand++;
    }
    else
    {
        reportStatus("ERROR", "command_execution_failed");
    }
}

void PatternExecutor::stop()
{
    currentSide = -1;
    currentCommand = -1;
    targetSide = -1;
    executingSingleSide = false;
    currentRow = 0;
    stopped = true;
    reportStatus("PATTERN_STOPPED", "");
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