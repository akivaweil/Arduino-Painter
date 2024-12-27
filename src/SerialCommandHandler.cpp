#include "SerialCommandHandler.h"

#include <Arduino.h>
#include <config.h>

#include "MaintenanceController.h"
#include "ServoController.h"

SerialCommandHandler::SerialCommandHandler(StateManager& state,
                                           MovementController& movement,
                                           HomingController& homing,
                                           PatternExecutor& pattern,
                                           MaintenanceController& maintenance,
                                           ServoController& servo)
    : stateManager(state),
      movementController(movement),
      homingController(homing),
      patternExecutor(pattern),
      maintenanceController(maintenance),
      servoController(servo)
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
    Serial.println(F("  GOTO <x> <y> - Absolute X Y movement"));
    Serial.println(F("  SPEED <side> <value> - Set speed for side (0-100)"));
    Serial.println(F("  ROTATE <degrees> - Rotate specified degrees (+ or -)"));
    Serial.println(F("  PRESSURE   - Toggle pressure pot on/off"));
    Serial.println(F("  PRIME_TIME <seconds> - Set prime duration"));
    Serial.println(F("  CLEAN_TIME <seconds> - Set clean duration"));
    Serial.println(F("  BACK_WASH  - Activate back wash"));
    Serial.println(F("  BACK_WASH_TIME <seconds> - Set back wash duration"));
    Serial.println(F("  SERVO <angle>    - Set servo angle (0-180)"));
    Serial.println(F("  SERVO_GET        - Get current servo angle"));
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
    if (command.startsWith("MOVE_") || command.startsWith("GOTO_") ||
        command.startsWith("GOTO"))  // Changed to catch both "GOTO" and "GOTO "
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

    // Handle manual movement commands separately
    if (command.startsWith("MANUAL_MOVE "))
    {
        handleContinuousMovement(command);
        return;
    }
    else if (command == "MANUAL_STOP")
    {
        handleManualStop();
        return;
    }

    // Handle manual movement commands separately
    if (command.startsWith("MANUAL_MOVE_DIAGONAL "))
    {
        handleContinuousDiagonalMovement(command);
        return;
    }
    else if (command == "SPRAY_START")
    {
        handleSprayToggle("START");
        return;
    }
    else if (command == "SPRAY_STOP")
    {
        handleSprayToggle("STOP");
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
        char buffer[64];
        snprintf(
            buffer, sizeof(buffer),
            "Can only rotate manually from IDLE or HOMED state (current: %s)",
            getStateString(currentState));
        sendResponse(false, buffer);
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
    SystemState currentState = stateManager.getCurrentState();
    if (currentState != IDLE && currentState != HOMED)
    {
        char buffer[64];
        snprintf(
            buffer, sizeof(buffer),
            "Can only move manually from IDLE or HOMED state (current: %s)",
            getStateString(currentState));
        sendResponse(false, buffer);
        return;
    }

    // Handle combined GOTO command
    if (command.startsWith("GOTO"))  // Changed to catch both "GOTO" and "GOTO "
    {
        if (command == "GOTO")  // Handle case with no parameters
        {
            sendResponse(false,
                         "Invalid GOTO command format. Use: GOTO <x> <y>");
            return;
        }

        // Parse two numbers after GOTO
        int firstSpace = command.indexOf(' ');
        int secondSpace = command.indexOf(' ', firstSpace + 1);

        // Debug logging
        Serial.println(F("=== GOTO Command Debug ==="));
        Serial.print(F("Raw command: '"));
        Serial.print(command);
        Serial.println(F("'"));
        Serial.print(F("First space at: "));
        Serial.println(firstSpace);
        Serial.print(F("Second space at: "));
        Serial.println(secondSpace);

        if (firstSpace == -1 || secondSpace == -1)
        {
            sendResponse(false,
                         "Invalid GOTO command format. Use: GOTO <x> <y>");
            return;
        }

        float xPos = command.substring(firstSpace + 1, secondSpace).toFloat();
        float yPos = command.substring(secondSpace + 1).toFloat();

        // Debug values
        Serial.print(F("Parsed X: "));
        Serial.println(xPos);
        Serial.print(F("Parsed Y: "));
        Serial.println(yPos);

        // Validate coordinates are within machine limits
        if (xPos < 0 || yPos < 0)
        {
            sendResponse(false, "Position values must be positive");
            return;
        }

        // Convert to steps for validation
        long xSteps = xPos * X_STEPS_PER_INCH;
        long ySteps = yPos * Y_STEPS_PER_INCH;

        // Validate against machine limits
        if (!movementController.isPositionValid(xSteps, ySteps))
        {
            sendResponse(false, "Position exceeds machine limits");
            return;
        }

        // Echo received command
        Serial.print(F("Manual movement: GOTO X="));
        Serial.print(xPos);
        Serial.print(F(" Y="));
        Serial.println(yPos);

        // Create and execute movement commands
        Command xMove('M', xPos, false);
        Command yMove('N', yPos, false);

        // Execute Y movement first to avoid potential collisions
        if (movementController.executeCommand(yMove))
        {
            if (movementController.executeCommand(xMove))
            {
                sendResponse(true, "Moving to position");
                return;
            }
        }

        sendResponse(false, "Failed to initiate movement");
        return;  // Make sure we return here
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
    const char* responseMsg = "";
    static char responseBuffer[128];  // Increased buffer size

    if (command == "START" || command == "FRONT" || command == "BACK" ||
        command == "LEFT" || command == "RIGHT")
    {
        if (currentState == HOMED)
        {
            // Check if pressure pot is active
            if (!maintenanceController.isPressurePotActive())
            {
                // Instead of rejecting, activate pressure pot and queue command
                maintenanceController.togglePressurePot();
                maintenanceController.queueDelayedCommand(command);
                sendResponse(true,
                             "Activating pressure pot, command will execute in "
                             "5 seconds");
                return;
            }
            // Get pressure pot activation time
            else if (maintenanceController.getPressurePotActiveTime() < 5000)
            {
                // If pot is active but not for 5 seconds, queue the command
                maintenanceController.queueDelayedCommand(command);
                sendResponse(
                    true,
                    "Waiting for pressure pot, command will execute shortly");
                return;
            }

            // Process the command
            if (command == "START")
            {
                patternExecutor.startPattern();
                stateManager.setState(EXECUTING_PATTERN);
                responseMsg = "Starting full pattern";
            }
            else
            {
                int side = (command == "FRONT")  ? 0
                           : (command == "BACK") ? 1
                           : (command == "LEFT") ? 2
                                                 : 3;
                patternExecutor.startSingleSide(side);
                stateManager.setState(PAINTING_SIDE);
                responseMsg = "Starting single side pattern";
            }
        }
        else
        {
            validCommand = false;
            snprintf(responseBuffer, sizeof(responseBuffer),
                     "Can only start painting from HOMED state (current: %s)",
                     getStateString(currentState));
            responseMsg = responseBuffer;
        }
    }
    else if (command == "HOME")
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
        responseMsg = "Starting homing sequence";
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
            snprintf(responseBuffer, sizeof(responseBuffer),
                     "Can only prime from IDLE or HOMED state (current: %s)",
                     getStateString(currentState));
            responseMsg = responseBuffer;
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
            snprintf(responseBuffer, sizeof(responseBuffer),
                     "Can only clean from IDLE or HOMED state (current: %s)",
                     getStateString(currentState));
            responseMsg = responseBuffer;
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
            snprintf(responseBuffer, sizeof(responseBuffer),
                     "Can only toggle pressure pot from IDLE or HOMED state "
                     "(current: %s)",
                     getStateString(currentState));
            responseMsg = responseBuffer;
        }
    }
    else if (command == "STOP")
    {
        // Ensure spray is off
        digitalWrite(PAINT_RELAY_PIN, LOW);

        movementController.stop();
        patternExecutor.stop();
        stateManager.setState(STOPPED);
        responseMsg = "stop activated";
    }
    else if (command.startsWith("PRIME_TIME "))
    {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex != -1)
        {
            unsigned long seconds = command.substring(spaceIndex + 1).toInt();
            if (seconds > 0 && seconds <= 30)  // Limit to reasonable range
            {
                maintenanceController.setPrimeDuration(seconds);
                responseMsg = "Prime duration updated";
            }
            else
            {
                validCommand = false;
                responseMsg = "Prime duration must be between 1 and 30 seconds";
            }
        }
    }
    else if (command.startsWith("CLEAN_TIME "))
    {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex != -1)
        {
            unsigned long seconds = command.substring(spaceIndex + 1).toInt();
            if (seconds > 0 && seconds <= 30)  // Limit to reasonable range
            {
                maintenanceController.setCleanDuration(seconds);
                responseMsg = "Clean duration updated";
            }
            else
            {
                validCommand = false;
                responseMsg = "Clean duration must be between 1 and 30 seconds";
            }
        }
    }
    else if (command.startsWith("SET_GRID "))
    {
        int spaceIndex = command.indexOf(' ', 9);
        if (spaceIndex != -1)
        {
            int x = command.substring(9, spaceIndex).toInt();
            int y = command.substring(spaceIndex + 1).toInt();
            patternExecutor.setGrid(x, y);
            responseMsg = "Grid dimensions updated";
        }
    }
    else if (command.startsWith("SET_HORIZONTAL_TRAVEL "))
    {
        int spaceIndex = command.indexOf(
            ' ', 22);  // Correct index after "SET_HORIZONTAL_TRAVEL "
        if (spaceIndex != -1)
        {
            // Debug: Show raw command
            Serial.print(F("Raw command: "));
            Serial.println(command);

            String xStr = command.substring(22, spaceIndex);
            String yStr = command.substring(spaceIndex + 1);

            // Debug: Show parsed strings
            Serial.print(F("Parsed strings - X: '"));
            Serial.print(xStr);
            Serial.print(F("' Y: '"));
            Serial.print(yStr);
            Serial.println(F("'"));

            float x = xStr.toFloat();
            float y = yStr.toFloat();

            // Debug log the received values
            Serial.println(F("=== SET_HORIZONTAL_TRAVEL Debug ==="));
            Serial.print(F("Input values - X: "));
            Serial.print(x);
            Serial.print(F(" Y: "));
            Serial.println(y);

            // Validate values
            if (x <= 0 || y <= 0)
            {
                Serial.println(
                    F("ERROR: Invalid travel distances (must be > 0)"));
                responseMsg = "Invalid travel distances";
                validCommand = false;
            }
            else
            {
                patternExecutor.setHorizontalTravel(x, y);
                responseMsg = "Horizontal travel distances updated";

                // Debug: Verify values were set
                Serial.println(F("Travel distances set successfully"));
            }
        }
        else
        {
            Serial.println(
                F("ERROR: Invalid format for SET_HORIZONTAL_TRAVEL"));
            validCommand = false;
            responseMsg = "Invalid command format";
        }
    }
    else if (command.startsWith("SET_VERTICAL_TRAVEL "))
    {
        int spaceIndex = command.indexOf(
            ' ', 20);  // Correct index after "SET_VERTICAL_TRAVEL "
        if (spaceIndex != -1)
        {
            // Debug: Show raw command
            Serial.print(F("Raw command: "));
            Serial.println(command);

            String xStr = command.substring(20, spaceIndex);
            String yStr = command.substring(spaceIndex + 1);

            // Debug: Show parsed strings
            Serial.print(F("Parsed strings - X: '"));
            Serial.print(xStr);
            Serial.print(F("' Y: '"));
            Serial.print(yStr);
            Serial.println(F("'"));

            float x = xStr.toFloat();
            float y = yStr.toFloat();

            // Debug log the received values
            Serial.println(F("=== SET_VERTICAL_TRAVEL Debug ==="));
            Serial.print(F("Input values - X: "));
            Serial.print(x);
            Serial.print(F(" Y: "));
            Serial.println(y);

            // Validate values
            if (x <= 0 || y <= 0)
            {
                Serial.println(
                    F("ERROR: Invalid travel distances (must be > 0)"));
                responseMsg = "Invalid travel distances";
                validCommand = false;
            }
            else
            {
                patternExecutor.setVerticalTravel(x, y);
                responseMsg = "Vertical travel distances updated";

                // Debug: Verify values were set
                Serial.println(F("Travel distances set successfully"));
            }
        }
        else
        {
            Serial.println(F("ERROR: Invalid format for SET_VERTICAL_TRAVEL"));
            validCommand = false;
            responseMsg = "Invalid command format";
        }
    }
    else if (command.startsWith("SET_OFFSET "))
    {
        // Format: SET_OFFSET <side> <x> <y>
        int firstSpace = command.indexOf(' ', 11);
        int secondSpace = command.indexOf(' ', firstSpace + 1);

        if (firstSpace != -1 && secondSpace != -1)
        {
            String side = command.substring(11, firstSpace);
            float x = command.substring(firstSpace + 1, secondSpace).toFloat();
            float y = command.substring(secondSpace + 1).toFloat();

            if (side == "FRONT")
            {
                patternExecutor.setFrontOffsets(x, y);
                responseMsg = "Front offsets updated";
            }
            else if (side == "BACK")
            {
                patternExecutor.setBackOffsets(x, y);
                responseMsg = "Back offsets updated";
            }
            else if (side == "LEFT")
            {
                patternExecutor.setLeftOffsets(x, y);
                responseMsg = "Left offsets updated";
            }
            else if (side == "RIGHT")
            {
                patternExecutor.setRightOffsets(x, y);
                responseMsg = "Right offsets updated";
            }
            else
            {
                validCommand = false;
                responseMsg = "Invalid side specified";
            }
        }
    }
    else if (command == "BACK_WASH")
    {
        if (currentState == IDLE || currentState == HOMED)
        {
            maintenanceController.startBackWash();
            stateManager.setState(BACK_WASHING);
            responseMsg = "Starting back wash sequence";
        }
        else
        {
            validCommand = false;
            snprintf(
                responseBuffer, sizeof(responseBuffer),
                "Can only back wash from IDLE or HOMED state (current: %s)",
                getStateString(currentState));
            responseMsg = responseBuffer;
        }
    }
    else if (command.startsWith("BACK_WASH_TIME "))
    {
        int spaceIndex = command.indexOf(' ');
        if (spaceIndex != -1)
        {
            unsigned long seconds = command.substring(spaceIndex + 1).toInt();
            if (seconds > 0 && seconds <= 30)  // Limit to reasonable range
            {
                maintenanceController.setBackWashDuration(seconds);
                responseMsg = "Back wash duration updated";
            }
            else
            {
                validCommand = false;
                responseMsg =
                    "Back wash duration must be between 1 and 30 seconds";
            }
        }
    }
    else if (command.startsWith("SERVO "))
    {
        handleServoCommand(command);
    }
    else if (command == "SERVO_GET")
    {
        int currentAngle = servoController.getCurrentAngle();
        snprintf(responseBuffer, sizeof(responseBuffer),
                 "Current servo angle: %d", currentAngle);
        responseMsg = responseBuffer;
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

// Add new helper method to convert state to string
const char* SerialCommandHandler::getStateString(SystemState state)
{
    switch (state)
    {
        case IDLE:
            return "IDLE";
        case HOMING_X:
            return "HOMING_X";
        case HOMING_Y:
            return "HOMING_Y";
        case HOMING_ROTATION:
            return "HOMING_ROTATION";
        case HOMED:
            return "HOMED";
        case EXECUTING_PATTERN:
            return "EXECUTING_PATTERN";
        case ERROR:
            return "ERROR";
        case CYCLE_COMPLETE:
            return "CYCLE_COMPLETE";
        case PRIMING:
            return "PRIMING";
        case CLEANING:
            return "CLEANING";
        case PAINTING_SIDE:
            return "PAINTING_SIDE";
        case STOPPED:
            return "STOPPED";
        case PAUSED:
            return "PAUSED";
        case MANUAL_ROTATING:
            return "MANUAL_ROTATING";
        case BACK_WASHING:
            return "BACK_WASHING";
        default:
            return "UNKNOWN";
    }
}

// Add new method to handle continuous movement
void SerialCommandHandler::handleContinuousMovement(const String& command)
{
    // Only allow manual movement in IDLE or HOMED state
    SystemState currentState = stateManager.getCurrentState();
    if (currentState != IDLE && currentState != HOMED)
    {
        char buffer[64];
        snprintf(
            buffer, sizeof(buffer),
            "Can only move manually from IDLE or HOMED state (current: %s)",
            getStateString(currentState));
        sendResponse(false, buffer);
        return;
    }

    // Parse command: MANUAL_MOVE <axis> <sign> <speed> <acceleration>
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    int thirdSpace = command.indexOf(' ', secondSpace + 1);
    int fourthSpace = command.indexOf(' ', thirdSpace + 1);

    if (firstSpace == -1 || secondSpace == -1 || thirdSpace == -1 ||
        fourthSpace == -1)
    {
        sendResponse(false, "Invalid manual move command format");
        return;
    }

    String axis = command.substring(firstSpace + 1, secondSpace);
    String sign = command.substring(secondSpace + 1, thirdSpace);
    float speed = command.substring(thirdSpace + 1, fourthSpace).toFloat();
    float acceleration = command.substring(fourthSpace + 1).toFloat();

    // Validate parameters
    if (axis != "X" && axis != "Y")
    {
        sendResponse(false, "Invalid axis specified");
        return;
    }

    if (sign != "+" && sign != "-")
    {
        sendResponse(false, "Invalid direction sign");
        return;
    }

    // Apply speed scaling (0-1 range to actual speeds)
    float scaledSpeed = (axis == "X" ? X_SPEED : Y_SPEED) * speed;
    float scaledAccel = (axis == "X" ? X_ACCEL : Y_ACCEL) * acceleration;

    // Start continuous movement
    if (movementController.startContinuousMovement(axis == "X", sign == "+",
                                                   scaledSpeed, scaledAccel))
    {
        sendResponse(true, "Manual movement started");
    }
    else
    {
        sendResponse(false, "Failed to start manual movement");
    }
}

void SerialCommandHandler::handleManualStop()
{
    movementController.stopMovement();
    sendResponse(true, "Manual movement stopped");
}

void SerialCommandHandler::handleContinuousDiagonalMovement(
    const String& command)
{
    // Only allow manual movement in IDLE or HOMED state
    SystemState currentState = stateManager.getCurrentState();
    if (currentState != IDLE && currentState != HOMED)
    {
        char buffer[64];
        snprintf(
            buffer, sizeof(buffer),
            "Can only move manually from IDLE or HOMED state (current: %s)",
            getStateString(currentState));
        sendResponse(false, buffer);
        return;
    }

    // Parse command: MANUAL_MOVE_DIAGONAL <X_SIGN> <Y_SIGN> <speed>
    // <acceleration>
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    int thirdSpace = command.indexOf(' ', secondSpace + 1);
    int fourthSpace = command.indexOf(' ', thirdSpace + 1);

    if (firstSpace == -1 || secondSpace == -1 || thirdSpace == -1 ||
        fourthSpace == -1)
    {
        sendResponse(false, "Invalid diagonal move command format");
        return;
    }

    String xDir = command.substring(firstSpace + 1, secondSpace);
    String yDir = command.substring(secondSpace + 1, thirdSpace);
    float speed = command.substring(thirdSpace + 1, fourthSpace).toFloat();
    float acceleration = command.substring(fourthSpace + 1).toFloat();

    // Validate parameters
    if ((xDir != "X+" && xDir != "X-") || (yDir != "Y+" && yDir != "Y-"))
    {
        sendResponse(false, "Invalid direction specified");
        return;
    }

    // Apply speed scaling (0-1 range to actual speeds)
    float scaledSpeed = X_SPEED * speed;  // Use X_SPEED as base for both axes
    float scaledAccel = X_ACCEL * acceleration;

    // Start continuous diagonal movement
    if (movementController.startContinuousDiagonalMovement(
            xDir == "X+", yDir == "Y+", scaledSpeed, scaledAccel))
    {
        sendResponse(true, "Diagonal movement started");
    }
    else
    {
        sendResponse(false, "Failed to start diagonal movement");
    }
}

void SerialCommandHandler::handleSprayToggle(const String& state)
{
    // Only allow spray toggle in IDLE or HOMED state
    SystemState currentState = stateManager.getCurrentState();
    if (currentState != IDLE && currentState != HOMED)
    {
        char buffer[64];
        snprintf(buffer, sizeof(buffer),
                 "Can only toggle spray from IDLE or HOMED state (current: %s)",
                 getStateString(currentState));
        sendResponse(false, buffer);
        return;
    }

    if (state == "START")
    {
        movementController.toggleSpray(true);
        sendResponse(true, "Spray started");
    }
    else if (state == "STOP")
    {
        movementController.toggleSpray(false);
        sendResponse(true, "Spray stopped");
    }
    else
    {
        sendResponse(false, "Invalid spray toggle state");
    }
}

void SerialCommandHandler::handleServoCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex == -1)
    {
        sendResponse(false, "Invalid servo command format");
        return;
    }

    int angle = command.substring(spaceIndex + 1).toInt();
    if (servoController.setAngle(angle))
    {
        char buffer[32];
        snprintf(buffer, sizeof(buffer), "Servo angle set to %d", angle);
        sendResponse(true, buffer);
    }
    else
    {
        sendResponse(false, "Invalid servo angle");
    }
}