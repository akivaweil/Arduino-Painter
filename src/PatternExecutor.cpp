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
      currentRow(0),
      currentPattern(nullptr)
{
}

PatternExecutor::~PatternExecutor() { delete[] currentPattern; }

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
            currentSide = -1;  // Reset currentSide
            targetSide = -1;

            // Start homing sequence
            homingController.startHoming();
            return;  // Important: return here to prevent pattern from
                     // restarting
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

                // // Add debug logging
                // Serial.println(F("=== Pattern Complete Rotation Debug ==="));
                // Serial.print(F("Current rotation steps: "));
                // Serial.println(movementController.getCurrentRotationSteps());
                long homePos = homingController.getHomeRotationPosition();
                // Serial.print(F("Home position steps: "));
                // Serial.println(homePos);
                // Serial.println(F("Sending absolute rotation command..."));

                // Return rotation to home position using absolute position
                Command rotateHome('R', homePos, true);
                movementController.executeCommand(rotateHome);

                // Add post-command debug
                Serial.print(F("Post-command rotation steps: "));
                Serial.println(movementController.getCurrentRotationSteps());

                // Wait for rotation to complete before homing X and Y
                if (!movementController.isMoving())
                {
                    Serial.println(F("Starting full homing sequence..."));
                    homingController.startHoming();
                    reportStatus("AUTO_HOMING", "starting_after_pattern");
                }
                else
                {
                    Serial.println(F("Waiting for rotation to complete..."));
                }
            }
            else
            {
                // Calculate rotation needed for next side
                int targetRotation = 0;
                switch (currentSide)
                {
                    case 1:  // Going to BACK
                        targetRotation = 180;
                        break;
                    case 2:  // Going to LEFT
                        targetRotation = 90;
                        break;
                    case 3:  // Going to RIGHT
                        targetRotation = 270;
                        break;
                }

                if (targetRotation != currentRotation)
                {
                    int optimizedRotation =
                        calculateOptimalRotation(targetRotation);

                    // // Debug logging
                    // Serial.println(F("=== Side Change Rotation Debug ==="));
                    // Serial.print(F("Target rotation: "));
                    // Serial.println(targetRotation);
                    // Serial.print(F("Current rotation: "));
                    // Serial.println(currentRotation);
                    // Serial.print(F("Optimized rotation: "));
                    // Serial.println(optimizedRotation);

                    Command rotateCmd('R', optimizedRotation, false);
                    movementController.executeCommand(rotateCmd);
                    currentRotation = targetRotation;
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
        currentRotation = 0;  // Reset rotation tracking when starting any side

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

        // Optimize rotation direction
        int optimizedRotation = calculateOptimalRotation(targetRotation);

        // // Add debug logging
        // Serial.println(F("=== Starting Single Side Rotation Debug ==="));
        // Serial.print(F("Target rotation: "));
        // Serial.println(targetRotation);
        // Serial.print(F("Optimized rotation: "));
        // Serial.println(optimizedRotation);
        // Serial.print(F("Current rotation: "));
        // Serial.println(currentRotation);

        // Execute optimized rotation
        Command rotateCmd('R', optimizedRotation, false);
        movementController.executeCommand(rotateCmd);
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
    return generatePattern(currentSide);
}

int PatternExecutor::getCurrentPatternSize() const
{
    return calculatePatternSize(currentSide);
}

void PatternExecutor::processNextCommand()
{
    Command* pattern = getCurrentPattern();
    if (pattern == nullptr)
    {
        reportStatus("ERROR", "invalid_pattern");
        return;
    }

    // Only apply pattern speed if we're still executing (not stopped)
    if (!stopped)
    {
        movementController.applyPatternSpeed(getCurrentPatternName());
    }

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

    // Clean up pattern
    delete[] currentPattern;
    currentPattern = nullptr;

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

int PatternExecutor::calculateOptimalRotation(int targetRotation)
{
    // Normalize current and target rotations to 0-359 range
    currentRotation = ((currentRotation % 360) + 360) % 360;
    targetRotation = ((targetRotation % 360) + 360) % 360;

    // Calculate both clockwise and counterclockwise distances
    int clockwiseDist = (targetRotation - currentRotation + 360) % 360;
    int counterclockwiseDist = (currentRotation - targetRotation + 360) % 360;

    // Choose the shorter rotation
    if (clockwiseDist <= counterclockwiseDist)
    {
        return clockwiseDist;
    }
    else
    {
        return -counterclockwiseDist;
    }
}

Command* PatternExecutor::generatePattern(int side) const
{
    // Calculate pattern size
    int size = calculatePatternSize(side);
    static Command pattern[100];  // Use a static array with a fixed size

    int idx = 0;

    // Initial movement to offset position
    pattern[idx++] = MOVETO_X(settings.offsets.x, false);
    Serial.print(F("Pattern Command: MOVETO_X to "));
    Serial.println(settings.offsets.x);

    float yOffset = settings.offsets.y;
    if (side == 1)
    {  // BACK
        yOffset =
            settings.travelDistance.y * settings.rows.y + settings.offsets.y;
    }
    pattern[idx++] = MOVETO_Y(yOffset, false);
    Serial.print(F("Pattern Command: MOVETO_Y to "));
    Serial.println(yOffset);

    // Generate rows
    int numRows = (side == 2 || side == 3) ? settings.rows.y : settings.rows.x;
    float travel = (side == 2 || side == 3) ? settings.travelDistance.x
                                            : settings.travelDistance.y;

    for (int row = 0; row < numRows; row++)
    {
        // Start spray
        pattern[idx++] = SPRAY_ON();
        Serial.println(F("Pattern Command: SPRAY_ON"));

        // Move across
        float dist = (side == 2 || side == 3) ? settings.travelDistance.x
                                              : settings.travelDistance.y;
        if (row % 2 == 1)
            dist = -dist;
        pattern[idx++] = MOVE_X(dist, true);
        Serial.print(F("Pattern Command: MOVE_X by "));
        Serial.println(dist);

        // Stop spray
        pattern[idx++] = SPRAY_OFF();
        Serial.println(F("Pattern Command: SPRAY_OFF"));

        // Move to next row if not last row
        if (row < numRows - 1)
        {
            float moveY = travel;
            if (side == 1 || side == 3)
                moveY = -moveY;
            pattern[idx++] = MOVE_Y(moveY, false);
            Serial.print(F("Pattern Command: MOVE_Y by "));
            Serial.println(moveY);
        }
    }

    return pattern;
}

int PatternExecutor::calculatePatternSize(int side) const
{
    int numRows = (side == 2 || side == 3) ? settings.rows.y : settings.rows.x;
    // Size = initial moves (2) + per row (spray on + move + spray off + move to
    // next row) * rows
    return 2 + (numRows * 4) -
           1;  // -1 because last row doesn't need vertical move
}