#include "SerialCommandHandler.h"

#include <Arduino.h>
#include <config.h>

SerialCommandHandler::SerialCommandHandler(StateManager& state,
                                           MovementController& movement,
                                           HomingController& homing,
                                           PatternExecutor& pattern,
                                           MaintenanceController& maintenance)
    : stateManager(state),
      movementController(movement),
      homingController(homing),
      patternExecutor(pattern),
      maintenanceController(maintenance)
{
}

void SerialCommandHandler::setup()
{
    Serial.println(F("Available Commands:"));
    Serial.println(F("  HOME       - Home machine"));
    Serial.println(F("  START      - Start full pattern"));
    Serial.println(F("  FRONT      - Paint front side"));
    Serial.println(F("  BACK       - Paint back side"));
    Serial.println(F("  LEFT       - Paint left side"));
    Serial.println(F("  RIGHT      - Paint right side"));
    Serial.println(F("  PRIME      - Prime spray gun"));
    Serial.println(F("  CLEAN      - Clean spray gun"));
    Serial.println(F("  CALIBRATE  - Quick calibration"));
    Serial.println(F("  STOP       - stop"));
    Serial.println(F("  MOVE_X <dist> - Relative X movement"));
    Serial.println(F("  MOVE_Y <dist> - Relative Y movement"));
    Serial.println(F("  GOTO_X <pos> - Absolute X movement"));
    Serial.println(F("  GOTO_Y <pos> - Absolute Y movement"));
    Serial.println(F("  SPEED <side> <value> - Set speed for side (0-100)"));
    Serial.println(F("  ROTATE <degrees> - Rotate specified degrees (+ or -)"));
    Serial.println(F("  PRESSURE   - Toggle pressure pot on/off"));
}

void SerialCommandHandler::processCommands()
{
    if (!Serial.available())
    {
        // Check if we need to transition out of MANUAL_ROTATING state
        if (stateManager.getCurrentState() == MANUAL_ROTATING &&
            !movementController.isMoving())
        {
            // Return to previous state (IDLE or HOMED)
            stateManager.setState(stateManager.getPreviousState());
        }
        return;
    }

    String command = Serial.readStringUntil('\n');
    command.trim();
    command.toUpperCase();

    // Handle movement commands separately
    if (command.startsWith("MOVE_") || command.startsWith("GOTO_"))
    {
        handleManualMovement(command);
        return;
    }

    // Handle speed commands separately
    if (command.startsWith("SPEED "))
    {
        handleSpeedCommand(command);
        return;
    }

    // Handle rotation command separately
    if (command.startsWith("ROTATE "))
    {
        handleRotationCommand(command);
        return;
    }

    // Process other system commands
    handleSystemCommand(command);
}

// Add new method to SerialCommandHandler.cpp:
void SerialCommandHandler::handleRotationCommand(const String& command)
{
    // Only allow manual rotation in IDLE or HOMED state
    SystemState currentState = stateManager.getCurrentState();
    if (currentState != IDLE && currentState != HOMED)
    {
        sendResponse(false,
                     "Can only rotate manually from IDLE or HOMED state");
        return;
    }

    // Parse rotation degrees from command
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex == -1)
    {
        sendResponse(false, "Invalid rotation command format");
        return;
    }

    float degrees = command.substring(spaceIndex + 1).toFloat();

    // Echo received command
    Serial.print(F("Manual rotation: "));
    Serial.print(degrees);
    Serial.println(F(" degrees"));

    // Create and execute rotation command
    Command rotateCmd('R', degrees, false);  // 'R' for rotation
    if (movementController.executeCommand(rotateCmd))
    {
        // Set state to MANUAL_ROTATING
        stateManager.setState(MANUAL_ROTATING);
        sendResponse(true, "Manual rotation started");
    }
    else
    {
        sendResponse(false, "Failed to execute rotation");
    }
}

// Modify SerialCommandHandler.cpp handleSpeedCommand
void SerialCommandHandler::handleSpeedCommand(const String& command)
{
    // Split command into parts (SPEED SIDE VALUE)
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);

    if (firstSpace == -1 || secondSpace == -1)
    {
        sendResponse(false, "Invalid speed command format");
        return;
    }

    String side = command.substring(firstSpace + 1, secondSpace);
    float value = command.substring(secondSpace + 1).toFloat();

    // Validate speed value (0-100)
    if (value < 0 || value > 100)
    {
        sendResponse(false, "Speed must be between 0 and 100");
        return;
    }

    // Store the speed for the specified pattern
    movementController.setPatternSpeed(side, value);
    sendResponse(true, "Pattern speed updated");
}

void SerialCommandHandler::handleManualMovement(const String& command)
{
    // Only allow manual movement in IDLE state
    if (stateManager.getCurrentState() != IDLE)
    {
        sendResponse(false, "Can only move manually from IDLE state");
        return;
    }

    // Split command into parts
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex == -1)
    {
        sendResponse(false, "Invalid movement command format");
        return;
    }

    String cmd = command.substring(0, spaceIndex);
    float distance = command.substring(spaceIndex + 1).toFloat();

    // Echo received command
    Serial.print(F("Manual movement: "));
    Serial.print(cmd);
    Serial.print(F(" = "));
    Serial.println(distance);

    // Convert command to single character for movement controller
    char moveType;
    if (cmd == "MOVE_X")
        moveType = 'X';
    else if (cmd == "MOVE_Y")
        moveType = 'Y';
    else if (cmd == "GOTO_X")
        moveType = 'M';
    else if (cmd == "GOTO_Y")
        moveType = 'N';
    else
    {
        sendResponse(false, "Invalid movement command");
        return;
    }

    // Create and execute movement command
    Command manualMove(moveType, distance,
                       false);  // No spray during manual movement
    if (movementController.executeCommand(manualMove))
    {
        sendResponse(true, "Manual movement started");
    }
    else
    {
        sendResponse(false, "Failed to execute movement");
    }
}

void SerialCommandHandler::handleSystemCommand(const String& command)
{
    SystemState currentState = stateManager.getCurrentState();
    bool validCommand = true;
    char responseBuffer[32];  // Buffer for formatting response messages
    const char* responseMsg = responseBuffer;

    if (command == "HOME")
    {
        // For HOME command, first handle like a STOP command if system is
        // executing
        if (currentState != IDLE && currentState != HOMED)
        {
            movementController.stop();
            patternExecutor.stop();
            stateManager.setState(STOPPED);
            Serial.println(F("Stopping current operation before homing"));
            delay(100);  // Brief delay to ensure stop is processed
        }

        // Now proceed with homing sequence
        homingController.startHoming();
        stateManager.setState(HOMING_X);
        responseMsg = "Starting homing sequence";
    }
    else if (command == "START")
    {
        if (currentState == HOMED)
        {
            patternExecutor.startPattern();
            stateManager.setState(EXECUTING_PATTERN);
            responseMsg = "Starting full pattern";
        }
        else
        {
            validCommand = false;
            responseMsg = "Can only start pattern from HOMED state";
        }
    }
    else if (command == "FRONT" || command == "BACK" || command == "LEFT" ||
             command == "RIGHT")
    {
        if (currentState == HOMED)
        {
            int side = (command == "FRONT")  ? 0
                       : (command == "BACK") ? 1
                       : (command == "LEFT") ? 2
                                             : 3;
            patternExecutor.startSingleSide(side);
            stateManager.setState(PAINTING_SIDE);
            responseMsg = "Starting single side pattern";
        }
        else
        {
            validCommand = false;
            responseMsg = "Can only start painting from HOMED state";
        }
    }
    else if (command == "PRIME")
    {
        if (currentState == IDLE || currentState == HOMED)
        {
            maintenanceController.startPriming();
            stateManager.setState(PRIMING);
            responseMsg = "Starting prime sequence";
        }
        else
        {
            validCommand = false;
            responseMsg = "Can only prime from IDLE state";
        }
    }
    else if (command == "CLEAN")
    {
        if (currentState == IDLE || currentState == HOMED)
        {
            maintenanceController.startCleaning();
            stateManager.setState(CLEANING);
            responseMsg = "Starting clean sequence";
        }
        else
        {
            validCommand = false;
            responseMsg = "Can only clean from IDLE state";
        }
    }
    else if (command == "PRESSURE")
    {
        if (currentState == IDLE || currentState == HOMED)
        {
            maintenanceController.togglePressurePot();
            responseMsg = maintenanceController.isPressurePotActive()
                              ? "Pressure pot activated"
                              : "Pressure pot deactivated";
        }
        else
        {
            validCommand = false;
            responseMsg =
                "Can only toggle pressure pot from IDLE or HOMED state";
        }
    }
    else if (command == "STOP")
    {
        movementController.stop();
        patternExecutor.stop();
        stateManager.setState(STOPPED);
        responseMsg = "stop activated";
    }
    else
    {
        validCommand = false;
        snprintf(responseBuffer, sizeof(responseBuffer),
                 "Unknown command: '%s'", command.c_str());
    }

    sendResponse(validCommand, responseMsg);
}

void SerialCommandHandler::sendResponse(bool success, const char* message)
{
    Serial.print(success ? F("OK: ") : F("WARNING: "));
    Serial.println(message);
}