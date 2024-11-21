#include "SerialCommandHandler.h"

#include <Arduino.h>

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
    Serial.println(F("  H/h - Home machine"));
    Serial.println(F("  S/s - Start full pattern"));
    Serial.println(F("  F/f - Paint front side"));
    Serial.println(F("  B/b - Paint back side"));
    Serial.println(F("  L/l - Paint left side"));
    Serial.println(F("  R/r - Paint right side"));
    Serial.println(F("  P/p - Prime spray gun"));
    Serial.println(F("  C/c - Clean spray gun"));
    Serial.println(F("  Q/q - Quick calibration"));
    Serial.println(F("  E/e - Emergency stop"));
    Serial.println(F("  x/y <dist> - Relative movement"));
    Serial.println(F("  m/n <pos> - Absolute movement"));
}

void SerialCommandHandler::processCommands()
{
    if (!Serial.available())
    {
        return;
    }

    char cmd = Serial.read();

    // Ignore whitespace characters
    if (isSpace(cmd))
    {
        return;
    }

    // Convert to uppercase for comparison
    char upperCmd = toupper(cmd);

    // Handle manual movement commands separately
    if (cmd == 'x' || cmd == 'y' || cmd == 'm' || cmd == 'n')
    {
        handleManualMovement(cmd);
        return;
    }

    // Process system commands
    handleSystemCommand(upperCmd);
}

void SerialCommandHandler::handleManualMovement(char cmd)
{
    // Only allow manual movement in IDLE state
    if (stateManager.getCurrentState() != IDLE)
    {
        sendResponse(false, "Can only move manually from IDLE state");
        return;
    }

    // Wait for numeric input
    delay(50);  // Small delay to allow buffer to fill
    String value = "";
    unsigned long startTime = millis();

    // Read until newline or timeout
    while (millis() - startTime < 1000)
    {  // 1 second timeout
        if (Serial.available())
        {
            char c = Serial.read();
            if (c == '\n' || c == '\r')
                break;
            if (isDigit(c) || c == '-' || c == '.')
                value += c;
        }
    }

    float distance = value.toFloat();

    // Echo received command
    Serial.print(F("Manual movement: "));
    Serial.print(cmd);
    Serial.print(F(" = "));
    Serial.println(distance);

    // Create and execute movement command
    Command manualMove(toupper(cmd), distance,
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

void SerialCommandHandler::handleSystemCommand(char cmd)
{
    SystemState currentState = stateManager.getCurrentState();
    bool validCommand = true;
    char responseBuffer[32];  // Buffer for formatting response messages
    const char* responseMsg = responseBuffer;

    switch (cmd)
    {
        case 'H':  // Home
            if (currentState == IDLE)
            {
                homingController.startHoming();
                stateManager.setState(HOMING_X);
                responseMsg = "Starting homing sequence";
            }
            else
            {
                validCommand = false;
                responseMsg = "Can only home from IDLE state";
            }
            break;

        case 'S':  // Start full pattern
            if (currentState == IDLE)
            {
                patternExecutor.startPattern();
                stateManager.setState(EXECUTING_PATTERN);
                responseMsg = "Starting full pattern";
            }
            else
            {
                validCommand = false;
                responseMsg = "Can only start pattern from IDLE state";
            }
            break;

        case 'F':  // Paint front
        case 'B':  // Paint back
        case 'L':  // Paint left
        case 'R':  // Paint right
        {
            if (currentState == IDLE)
            {
                int side = (cmd == 'F')   ? 0
                           : (cmd == 'B') ? 1
                           : (cmd == 'L') ? 2
                                          : 3;
                patternExecutor.startSingleSide(side);
                stateManager.setState(PAINTING_SIDE);
                responseMsg = "Starting single side pattern";
            }
            else
            {
                validCommand = false;
                responseMsg = "Can only start painting from IDLE state";
            }
            break;
        }

        case 'P':  // Prime gun
            if (currentState == IDLE)
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
            break;

        case 'C':  // Clean gun
            if (currentState == IDLE)
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
            break;

        case 'Q':  // Quick calibration
            if (currentState == IDLE)
            {
                maintenanceController.startCalibration();
                stateManager.setState(CALIBRATING);
                responseMsg = "Starting calibration";
            }
            else
            {
                validCommand = false;
                responseMsg = "Can only calibrate from IDLE state";
            }
            break;

        case 'E':  // Emergency stop
            movementController.emergencyStop();
            stateManager.setState(ERROR);
            responseMsg = "Emergency stop activated";
            break;

        default:
            validCommand = false;
            snprintf(responseBuffer, sizeof(responseBuffer),
                     "Unknown command: '%c'", cmd);
            break;
    }

    sendResponse(validCommand, responseMsg);
}

void SerialCommandHandler::sendResponse(bool success, const char* message)
{
    Serial.print(success ? F("OK: ") : F("WARNING: "));
    Serial.println(message);
}