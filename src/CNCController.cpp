// CNCController.cpp
#include "CNCController.h"

#include "Patterns.h"
#include "config.h"

CNCController::CNCController()
    : systemState(IDLE),
      motorsRunning(false),
      currentSide(0),
      currentCommand(0),
      lastExecutedCommand(),  // Initialize with default constructor
      awaitingCompletion(false),
      stepperX(AccelStepper::DRIVER, X_STEP_PIN, X_DIR_PIN),
      stepperY(AccelStepper::DRIVER, Y_STEP_PIN, Y_DIR_PIN),
      stepperRotation(AccelStepper::DRIVER, ROTATION_STEP_PIN, ROTATION_DIR_PIN)
{
}

void CNCController::setup()
{
    Serial.begin(115200);

    pinMode(PAINT_RELAY_PIN, OUTPUT);
    digitalWrite(PAINT_RELAY_PIN, HIGH);

    pinMode(X_HOME_SENSOR_PIN, INPUT_PULLUP);
    pinMode(Y_HOME_SENSOR_PIN, INPUT_PULLUP);

    xHomeSensor.attach(X_HOME_SENSOR_PIN);
    yHomeSensor.attach(Y_HOME_SENSOR_PIN);
    xHomeSensor.interval(20);
    yHomeSensor.interval(20);

    stepperX.setMaxSpeed(X_SPEED);
    stepperX.setAcceleration(X_ACCEL);
    stepperX.setPinsInverted(true);

    stepperY.setMaxSpeed(Y_SPEED);
    stepperY.setAcceleration(Y_ACCEL);
    stepperY.setPinsInverted(false);

    stepperRotation.setMaxSpeed(ROTATION_SPEED);
    stepperRotation.setAcceleration(ROTATION_ACCEL);

    Serial.println(F("CNC Paint Sprayer Ready"));
    Serial.println(F("Commands: H-Home S-Start E-Stop R-Reset"));
}

void CNCController::executeCommand(const Command& cmd)
{
    // Send acknowledgment that command is being executed
    Serial.print(F("Executing command: "));
    Serial.print(cmd.type);
    Serial.print(F(" Value: "));
    Serial.print(cmd.value);
    Serial.print(F(" Spray: "));
    Serial.println(cmd.sprayOn ? "ON" : "OFF");

    switch (cmd.type)
    {
        case 'X':
            if (cmd.sprayOn)
                digitalWrite(PAINT_RELAY_PIN, LOW);
            stepperX.move(cmd.value * X_STEPS_PER_INCH);
            break;

        case 'Y':
            if (cmd.sprayOn)
                digitalWrite(PAINT_RELAY_PIN, LOW);
            stepperY.move(cmd.value * Y_STEPS_PER_INCH);
            break;

        case 'R':
            stepperRotation.move((cmd.value * STEPS_PER_ROTATION) / 360);
            break;

        case 'S':
            digitalWrite(PAINT_RELAY_PIN, cmd.sprayOn ? LOW : HIGH);
            break;

        default:
            Serial.println(F("ERROR: Invalid command type"));
            return;
    }

    // Track the command for completion feedback
    lastExecutedCommand = cmd;
    awaitingCompletion = true;
}

void CNCController::processPattern()
{
    if (!motorsRunning)
    {
        // If we were waiting for command completion, send feedback
        if (awaitingCompletion)
        {
            Serial.print(F("Command complete: "));
            Serial.print(lastExecutedCommand.type);
            Serial.print(F(" at position X:"));
            Serial.print(stepperX.currentPosition());
            Serial.print(F(" Y:"));
            Serial.println(stepperY.currentPosition());
            awaitingCompletion = false;
        }

        Command* currentPattern;
        int patternSize;

        switch (currentSide)
        {
            case 0:
                currentPattern = SIDE1_PATTERN;
                patternSize = SIDE1_SIZE;
                break;
            case 1:
                currentPattern = SIDE2_PATTERN;
                patternSize = SIDE2_SIZE;
                break;
            case 2:
                currentPattern = SIDE3_PATTERN;
                patternSize = SIDE3_SIZE;
                break;
            case 3:
                currentPattern = SIDE4_PATTERN;
                patternSize = SIDE4_SIZE;
                break;
        }

        if (currentCommand < patternSize)
        {
            executeCommand(currentPattern[currentCommand]);
            currentCommand++;
        }
        else
        {
            currentCommand = 0;
            currentSide++;
            if (currentSide >= 4)
            {
                systemState = CYCLE_COMPLETE;
            }
        }
    }
}

void CNCController::handleSerialCommand()
{
    if (Serial.available())
    {
        char cmd = Serial.read();

        // For manual movement commands, we need to read the value
        if (cmd == 'x' || cmd == 'y')
        {
            // Wait for numeric input
            delay(50);  // Small delay to allow buffer to fill
            String value = "";

            // Read until newline or timeout
            unsigned long startTime = millis();
            while (millis() - startTime < 1000)  // 1 second timeout
            {
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

            // Echo received command and validate
            Serial.print(F("Manual movement: "));
            Serial.print(cmd);
            Serial.print(F(" = "));
            Serial.println(distance);

            bool validCommand = true;
            String cmdDescription;

            if (systemState == IDLE)
            {
                Command manualMove(
                    toupper(cmd),  // Convert to uppercase for Command type
                    distance,      // Distance in inches
                    false          // No spray during manual movement
                );
                executeCommand(manualMove);
                cmdDescription = F("Executing manual movement");
            }
            else
            {
                validCommand = false;
                cmdDescription = F("Can only move manually from IDLE state");
            }

            // Send command validation status
            Serial.print(validCommand ? F("OK: ") : F("ERROR: "));
            Serial.println(cmdDescription);
            return;
        }

        // Echo received command and validate
        Serial.print(F("Received command: "));
        Serial.println(cmd);

        bool validCommand = true;
        String cmdDescription;

        switch (cmd)
        {
            case 'H':
            case 'h':
                if (systemState == IDLE)
                {
                    systemState = HOMING_X;
                    cmdDescription = F("Starting homing sequence");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only home from IDLE state");
                }
                break;

            case 'S':
            case 's':
                if (systemState == IDLE)
                {
                    currentSide = 0;
                    currentCommand = 0;
                    systemState = EXECUTING_PATTERN;
                    cmdDescription = F("Starting pattern execution");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only start from IDLE state");
                }
                break;

            case 'E':
            case 'e':
                systemState = ERROR;
                digitalWrite(PAINT_RELAY_PIN, HIGH);
                stepperX.stop();
                stepperY.stop();
                stepperRotation.stop();
                cmdDescription = F("Emergency stop activated");
                break;

            case 'R':
            case 'r':
                if (systemState == ERROR)
                {
                    systemState = IDLE;
                    cmdDescription = F("System reset to IDLE");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only reset from ERROR state");
                }
                break;

            default:
                validCommand = false;
                cmdDescription = F("Unknown command");
                break;
        }

        // Send command validation status
        Serial.print(validCommand ? F("OK: ") : F("ERROR: "));
        Serial.println(cmdDescription);
    }
}

void CNCController::processSystemState()
{
    SystemState previousState = systemState;

    switch (systemState)
    {
        case IDLE:
            break;

        case HOMING_X:
            if (!xHomeSensor.read())
            {
                stepperX.setCurrentPosition(0);
                systemState = HOMING_Y;
                Serial.println(F("X-axis homing complete"));
            }
            else if (!stepperX.isRunning())
            {
                stepperX.moveTo(-1000000);
            }
            break;

        case HOMING_Y:
            if (!yHomeSensor.read())
            {
                stepperY.setCurrentPosition(0);
                systemState = EXECUTING_PATTERN;
                Serial.println(F("Y-axis homing complete"));
            }
            else if (!stepperY.isRunning())
            {
                stepperY.moveTo(-1000000);
            }
            break;

        case EXECUTING_PATTERN:
            processPattern();
            break;

        case ERROR:
            break;

        case CYCLE_COMPLETE:
            if (!motorsRunning)
            {
                Serial.println(F("Cycle complete"));
                systemState = IDLE;
            }
            break;
    }

    // Report state changes
    if (previousState != systemState)
    {
        Serial.print(F("State changed: "));
        switch (systemState)
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
            case CYCLE_COMPLETE:
                Serial.println(F("CYCLE_COMPLETE"));
                break;
        }
    }
}

void CNCController::updateMotors()
{
    // Update all stepper positions
    stepperX.run();
    stepperY.run();
    stepperRotation.run();

    // Check if any motors are currently running
    motorsRunning = stepperX.isRunning() || stepperY.isRunning() ||
                    stepperRotation.isRunning();

    // If motors just stopped running, turn off spray
    if (!motorsRunning &&
        (lastExecutedCommand.type == 'X' || lastExecutedCommand.type == 'Y'))
    {
        digitalWrite(PAINT_RELAY_PIN, HIGH);  // Turn off spray
    }
}

void CNCController::loop()
{
    xHomeSensor.update();
    yHomeSensor.update();

    updateMotors();
    handleSerialCommand();
    processSystemState();
}