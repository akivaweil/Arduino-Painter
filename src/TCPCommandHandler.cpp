#include "TCPCommandHandler.h"

#include "config.h"

TCPCommandHandler::TCPCommandHandler(StateManager& state,
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
      servoController(servo),
      server(WiFiServer(0))  // Port will be set in setup
{
}

void TCPCommandHandler::setup(const char* ssid, const char* password, int port)
{
    // Connect to WiFi
    Serial.print("Connecting to WiFi");
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);
        Serial.print(".");
    }

    Serial.println("\nWiFi connected");
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());

    // Start TCP server
    server = WiFiServer(port);
    server.begin();
    Serial.print("TCP server started on port ");
    Serial.println(port);
}

void TCPCommandHandler::processConnections()
{
    if (!client || !client.connected())
    {
        client = server.accept();  // Non-blocking accept
    }

    if (client && client.available())
    {
        String command = client.readStringUntil('\n');
        command.trim();
        command.toUpperCase();
        handleSystemCommand(command);
    }
}

void TCPCommandHandler::sendJsonResponse(bool success, const char* message)
{
    StaticJsonDocument<200> doc;
    doc["success"] = success;
    doc["message"] = message;

    String response;
    serializeJson(doc, response);
    response += "\n";

    if (client && client.connected())
    {
        client.print(response);
    }
}

void TCPCommandHandler::sendResponse(bool success, const char* message)
{
    if (!client || !client.connected())
        return;

    client.print(success ? '+' : '-');
    client.println(message);
}

void TCPCommandHandler::handleManualMovement(const String& command)
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
    if (command.startsWith("GOTO"))
    {
        if (command == "GOTO")
        {
            sendResponse(false,
                         "Invalid GOTO command format. Use: GOTO <x> <y>");
            return;
        }

        // Parse two numbers after GOTO
        int firstSpace = command.indexOf(' ');
        int secondSpace = command.indexOf(' ', firstSpace + 1);

        if (firstSpace == -1 || secondSpace == -1)
        {
            sendResponse(false,
                         "Invalid GOTO command format. Use: GOTO <x> <y>");
            return;
        }

        float xPos = command.substring(firstSpace + 1, secondSpace).toFloat();
        float yPos = command.substring(secondSpace + 1).toFloat();

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

        // Create and execute movement commands
        Command xMove('M', xPos, false);
        Command yMove('N', yPos, false);

        if (movementController.executeCommand(yMove) &&
            movementController.executeCommand(xMove))
        {
            sendResponse(true, "Moving to position");
            return;
        }

        sendResponse(false, "Failed to initiate movement");
        return;
    }

    // Handle other movement commands
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex == -1)
    {
        sendResponse(false, "Invalid movement command format");
        return;
    }

    String cmd = command.substring(0, spaceIndex);
    float distance = command.substring(spaceIndex + 1).toFloat();

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

    Command manualMove(moveType, distance, false);
    if (movementController.executeCommand(manualMove))
    {
        sendResponse(true, "Manual movement started");
    }
    else
    {
        sendResponse(false, "Failed to execute movement");
    }
}

void TCPCommandHandler::handleSpeedCommand(const String& command)
{
    int firstSpace = command.indexOf(' ');
    int secondSpace = command.indexOf(' ', firstSpace + 1);

    if (firstSpace == -1 || secondSpace == -1)
    {
        sendResponse(false, "Invalid speed command format");
        return;
    }

    String side = command.substring(firstSpace + 1, secondSpace);
    float value = command.substring(secondSpace + 1).toFloat();

    if (value < 0 || value > 100)
    {
        sendResponse(false, "Speed must be between 0 and 100");
        return;
    }

    movementController.setPatternSpeed(side, value);
    sendResponse(true, "Pattern speed updated");
}

void TCPCommandHandler::handleRotationCommand(const String& command)
{
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

    int spaceIndex = command.indexOf(' ');
    if (spaceIndex == -1)
    {
        sendResponse(false, "Invalid rotation command format");
        return;
    }

    float degrees = command.substring(spaceIndex + 1).toFloat();
    degrees = -degrees;  // Invert for clockwise rotation

    Command rotateCmd('R', degrees, false);
    if (movementController.executeCommand(rotateCmd))
    {
        stateManager.setState(MANUAL_ROTATING);
        sendResponse(true, "Manual rotation started");
    }
    else
    {
        sendResponse(false, "Failed to execute rotation");
    }
}

const char* TCPCommandHandler::getStateString(SystemState state)
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

void TCPCommandHandler::handleSystemCommand(const String& command)
{
    SystemState currentState = stateManager.getCurrentState();
    bool validCommand = true;
    const char* responseMsg = "";
    static char responseBuffer[128];

    if (command == "START" || command == "FRONT" || command == "BACK" ||
        command == "LEFT" || command == "RIGHT")
    {
        if (currentState == HOMED)
        {
            if (!maintenanceController.isPressurePotActive())
            {
                maintenanceController.togglePressurePot();
                maintenanceController.queueDelayedCommand(command);
                sendResponse(true,
                             "Activating pressure pot, command will execute in "
                             "5 seconds");
                return;
            }
            else if (maintenanceController.getPressurePotActiveTime() < 5000)
            {
                maintenanceController.queueDelayedCommand(command);
                sendResponse(
                    true,
                    "Waiting for pressure pot, command will execute shortly");
                return;
            }

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
        // First handle like a STOP command if system is executing
        if (currentState != IDLE && currentState != HOMED)
        {
            movementController.stop();
            patternExecutor.stop();
            stateManager.setState(STOPPED);
            delay(100);  // Brief delay to ensure stop is processed
        }

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
        digitalWrite(PAINT_RELAY_PIN, LOW);
        movementController.stop();
        patternExecutor.stop();
        stateManager.setState(STOPPED);
        responseMsg = "Stop activated";
    }
    else if (command.startsWith("PRIME_TIME "))
    {
        handlePrimeTimeCommand(command);
        return;
    }
    else if (command.startsWith("CLEAN_TIME "))
    {
        handleCleanTimeCommand(command);
        return;
    }
    else if (command.startsWith("SET_GRID "))
    {
        handleSetGridCommand(command);
        return;
    }
    else if (command.startsWith("SET_HORIZONTAL_TRAVEL "))
    {
        handleSetHorizontalTravelCommand(command);
        return;
    }
    else if (command.startsWith("SET_VERTICAL_TRAVEL "))
    {
        handleSetVerticalTravelCommand(command);
        return;
    }
    else if (command.startsWith("SET_OFFSET "))
    {
        handleSetOffsetCommand(command);
        return;
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
        handleBackWashTimeCommand(command);
        return;
    }
    else if (command.startsWith("SERVO "))
    {
        handleServoCommand(command);
        return;
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
        responseMsg = responseBuffer;
    }

    sendResponse(validCommand, responseMsg);
}

void TCPCommandHandler::handlePrimeTimeCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1)
    {
        unsigned long seconds = command.substring(spaceIndex + 1).toInt();
        if (seconds > 0 && seconds <= 30)
        {
            maintenanceController.setPrimeDuration(seconds);
            sendResponse(true, "Prime duration updated");
        }
        else
        {
            sendResponse(false,
                         "Prime duration must be between 1 and 30 seconds");
        }
    }
}

void TCPCommandHandler::handleCleanTimeCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1)
    {
        unsigned long seconds = command.substring(spaceIndex + 1).toInt();
        if (seconds > 0 && seconds <= 30)
        {
            maintenanceController.setCleanDuration(seconds);
            sendResponse(true, "Clean duration updated");
        }
        else
        {
            sendResponse(false,
                         "Clean duration must be between 1 and 30 seconds");
        }
    }
}

void TCPCommandHandler::handleSetGridCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ', 9);
    if (spaceIndex != -1)
    {
        int x = command.substring(9, spaceIndex).toInt();
        int y = command.substring(spaceIndex + 1).toInt();
        patternExecutor.setGrid(x, y);
        sendResponse(true, "Grid dimensions updated");
    }
    else
    {
        sendResponse(false, "Invalid SET_GRID command format");
    }
}

void TCPCommandHandler::handleSetHorizontalTravelCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ', 22);
    if (spaceIndex != -1)
    {
        String xStr = command.substring(22, spaceIndex);
        String yStr = command.substring(spaceIndex + 1);
        float x = xStr.toFloat();
        float y = yStr.toFloat();

        if (x <= 0 || y <= 0)
        {
            sendResponse(false, "Invalid travel distances (must be > 0)");
        }
        else
        {
            patternExecutor.setHorizontalTravel(x, y);
            sendResponse(true, "Horizontal travel distances updated");
        }
    }
    else
    {
        sendResponse(false, "Invalid SET_HORIZONTAL_TRAVEL command format");
    }
}

void TCPCommandHandler::handleSetVerticalTravelCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ', 20);
    if (spaceIndex != -1)
    {
        String xStr = command.substring(20, spaceIndex);
        String yStr = command.substring(spaceIndex + 1);
        float x = xStr.toFloat();
        float y = yStr.toFloat();

        if (x <= 0 || y <= 0)
        {
            sendResponse(false, "Invalid travel distances (must be > 0)");
        }
        else
        {
            patternExecutor.setVerticalTravel(x, y);
            sendResponse(true, "Vertical travel distances updated");
        }
    }
    else
    {
        sendResponse(false, "Invalid SET_VERTICAL_TRAVEL command format");
    }
}

void TCPCommandHandler::handleSetOffsetCommand(const String& command)
{
    int firstSpace = command.indexOf(' ', 11);
    int secondSpace = command.indexOf(' ', firstSpace + 1);
    int thirdSpace = command.indexOf(' ', secondSpace + 1);

    if (firstSpace != -1 && secondSpace != -1 && thirdSpace != -1)
    {
        String side = command.substring(11, firstSpace);
        float x = command.substring(firstSpace + 1, secondSpace).toFloat();
        float y = command.substring(secondSpace + 1, thirdSpace).toFloat();
        float angle = command.substring(thirdSpace + 1).toFloat();

        if (side == "FRONT")
        {
            patternExecutor.setFrontOffsets(x, y, angle);
            sendResponse(true, "Front offsets updated");
        }
        else if (side == "BACK")
        {
            patternExecutor.setBackOffsets(x, y, angle);
            sendResponse(true, "Back offsets updated");
        }
        else if (side == "LEFT")
        {
            patternExecutor.setLeftOffsets(x, y, angle);
            sendResponse(true, "Left offsets updated");
        }
        else if (side == "RIGHT")
        {
            patternExecutor.setRightOffsets(x, y, angle);
            sendResponse(true, "Right offsets updated");
        }
        else
        {
            sendResponse(false, "Invalid side specified");
        }
    }
    else
    {
        sendResponse(false, "Invalid SET_OFFSET command format");
    }
}

void TCPCommandHandler::handleBackWashTimeCommand(const String& command)
{
    int spaceIndex = command.indexOf(' ');
    if (spaceIndex != -1)
    {
        unsigned long seconds = command.substring(spaceIndex + 1).toInt();
        if (seconds > 0 && seconds <= 30)
        {
            maintenanceController.setBackWashDuration(seconds);
            sendResponse(true, "Back wash duration updated");
        }
        else
        {
            sendResponse(false,
                         "Back wash duration must be between 1 and 30 seconds");
        }
    }
}

void TCPCommandHandler::handleServoCommand(const String& command)
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