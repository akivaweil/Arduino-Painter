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

        // Get the pattern for the current side
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

        // Execute the current command in the pattern
        if (currentCommand < patternSize)
        {
            executeCommand(currentPattern[currentCommand]);
            currentCommand++;
        }
        else
        {
            // Side is complete
            currentCommand = 0;
            if (systemState == EXECUTING_PATTERN)
            {
                // If we're executing the full pattern, move to next side
                currentSide++;
                if (currentSide >= 4)
                {
                    systemState = CYCLE_COMPLETE;
                }
            }
            else
            {
                // If we're just doing one side, we're done
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
            // ... existing manual movement handling ...
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

            // New commands
            case 'P':
            case 'p':
                if (systemState == IDLE)
                {
                    digitalWrite(PAINT_RELAY_PIN, LOW);   // Turn on spray
                    delay(5000);                          // Prime for 5 seconds
                    digitalWrite(PAINT_RELAY_PIN, HIGH);  // Turn off spray
                    cmdDescription = F("Gun primed");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only prime from IDLE state");
                }
                break;

            case 'C':
            case 'c':
                if (systemState == IDLE)
                {
                    digitalWrite(PAINT_RELAY_PIN, LOW);  // Turn on spray
                    delay(10000);                        // Clean for 10 seconds
                    digitalWrite(PAINT_RELAY_PIN, HIGH);  // Turn off spray
                    delay(5000);                          // Wait 5 seconds
                    cmdDescription = F("Gun cleaned");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only clean from IDLE state");
                }
                break;

            case 'Q':
            case 'q':
                if (systemState == IDLE)
                {
                    // Quick calibration sequence
                    systemState = HOMING_X;  // Start with homing
                    cmdDescription = F("Starting quick calibration");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only calibrate from IDLE state");
                }
                break;

            // Individual side painting commands
            case 'F':
            case 'f':
                if (systemState == IDLE)
                {
                    currentSide = 0;
                    currentCommand = 0;
                    systemState = EXECUTING_PATTERN;
                    cmdDescription = F("Painting front side");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only paint side from IDLE state");
                }
                break;

            case 'I':
            case 'i':
                if (systemState == IDLE)
                {
                    currentSide = 1;
                    currentCommand = 0;
                    systemState = EXECUTING_PATTERN;
                    cmdDescription = F("Painting right side");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only paint side from IDLE state");
                }
                break;

            case 'B':
            case 'b':
                if (systemState == IDLE)
                {
                    currentSide = 2;
                    currentCommand = 0;
                    systemState = EXECUTING_PATTERN;
                    cmdDescription = F("Painting back side");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only paint side from IDLE state");
                }
                break;

            case 'L':
            case 'l':
                if (systemState == IDLE)
                {
                    currentSide = 3;
                    currentCommand = 0;
                    systemState = EXECUTING_PATTERN;
                    cmdDescription = F("Painting left side");
                }
                else
                {
                    validCommand = false;
                    cmdDescription = F("Can only paint side from IDLE state");
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

        case PRIMING:
            executePrimeSequence();
            break;

        case CLEANING:
            executeCleanSequence();
            break;

        case CALIBRATING:
            executeCalibrationSequence();
            break;

        case PAINTING_SIDE:
            executeSingleSidePaint(targetSide);
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
            case PRIMING:
                Serial.println(F("PRIMING"));
                break;
            case CLEANING:
                Serial.println(F("CLEANING"));
                break;
            case CALIBRATING:
                Serial.println(F("CALIBRATING"));
                break;
            case PAINTING_SIDE:
                Serial.println(F("PAINTING_SIDE"));
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

void CNCController::primeGun()
{
    if (systemState == IDLE)
    {
        systemState = PRIMING;
        Serial.println(F("Starting gun priming sequence"));
    }
    else
    {
        Serial.println(F("ERROR: Can only prime from IDLE state"));
    }
}

void CNCController::cleanGun()
{
    if (systemState == IDLE)
    {
        systemState = CLEANING;
        Serial.println(F("Starting gun cleaning sequence"));
    }
    else
    {
        Serial.println(F("ERROR: Can only clean from IDLE state"));
    }
}

void CNCController::quickCalibrate()
{
    if (systemState == IDLE)
    {
        systemState = CALIBRATING;
        Serial.println(F("Starting quick calibration"));
    }
    else
    {
        Serial.println(F("ERROR: Can only calibrate from IDLE state"));
    }
}

void CNCController::paintSide(int side)
{
    if (systemState == IDLE && side >= 0 && side < 4)
    {
        systemState = PAINTING_SIDE;
        targetSide = side;
        currentCommand = 0;
        Serial.print(F("Starting single side paint for side "));
        Serial.println(side);
    }
    else
    {
        Serial.println(
            F("ERROR: Invalid state or side for single side painting"));
    }
}

void CNCController::executePrimeSequence()
{
    static int primeStep = 0;
    static unsigned long primeTimer = 0;

    switch (primeStep)
    {
        case 0:                                  // Start sequence
            digitalWrite(PAINT_RELAY_PIN, LOW);  // Turn on spray
            primeTimer = millis();
            primeStep++;
            break;

        case 1:  // Prime for 5 seconds
            if (millis() - primeTimer >= 5000)
            {
                digitalWrite(PAINT_RELAY_PIN, HIGH);  // Turn off spray
                primeStep = 0;
                systemState = IDLE;
                Serial.println(F("Prime sequence complete"));
            }
            break;
    }
}

void CNCController::executeCleanSequence()
{
    static int cleanStep = 0;
    static unsigned long cleanTimer = 0;

    switch (cleanStep)
    {
        case 0:                                  // Start cleaning
            digitalWrite(PAINT_RELAY_PIN, LOW);  // Turn on spray
            cleanTimer = millis();
            cleanStep++;
            break;

        case 1:  // Spray cleaner for 10 seconds
            if (millis() - cleanTimer >= 10000)
            {
                digitalWrite(PAINT_RELAY_PIN, HIGH);  // Turn off spray
                cleanTimer = millis();
                cleanStep++;
            }
            break;

        case 2:  // Wait 5 seconds
            if (millis() - cleanTimer >= 5000)
            {
                cleanStep = 0;
                systemState = IDLE;
                Serial.println(F("Clean sequence complete"));
            }
            break;
    }
}

void CNCController::executeCalibrationSequence()
{
    static int calStep = 0;
    static unsigned long calTimer = 0;

    switch (calStep)
    {
        case 0:                      // Home axes
            systemState = HOMING_X;  // Start with homing
            calStep++;
            break;

        case 1:  // Wait for homing to complete
            if (systemState == EXECUTING_PATTERN)
            {  // Homing complete
                // Move to calibration position
                stepperX.moveTo(X_STEPS_PER_INCH * 2);  // Move 2 inches X
                stepperY.moveTo(Y_STEPS_PER_INCH * 2);  // Move 2 inches Y
                calTimer = millis();
                calStep++;
            }
            break;

        case 2:  // Short spray test
            if (!motorsRunning && millis() - calTimer >= 1000)
            {
                digitalWrite(PAINT_RELAY_PIN, LOW);  // Turn on spray
                calTimer = millis();
                calStep++;
            }
            break;

        case 3:  // Complete calibration
            if (millis() - calTimer >= 2000)
            {
                digitalWrite(PAINT_RELAY_PIN, HIGH);  // Turn off spray
                calStep = 0;
                systemState = IDLE;
                Serial.println(F("Calibration complete"));
            }
            break;
    }
}

void CNCController::executeSingleSidePaint(int side)
{
    Command* pattern;
    int patternSize;

    // Select pattern based on side
    switch (side)
    {
        case 0:
            pattern = SIDE1_PATTERN;
            patternSize = SIDE1_SIZE;
            break;
        case 1:
            pattern = SIDE2_PATTERN;
            patternSize = SIDE2_SIZE;
            break;
        case 2:
            pattern = SIDE3_PATTERN;
            patternSize = SIDE3_SIZE;
            break;
        case 3:
            pattern = SIDE4_PATTERN;
            patternSize = SIDE4_SIZE;
            break;
        default:
            systemState = ERROR;
            return;
    }

    // Execute the pattern for just this side
    if (!motorsRunning && currentCommand < patternSize)
    {
        executeCommand(pattern[currentCommand]);
        currentCommand++;
    }
    else if (!motorsRunning && currentCommand >= patternSize)
    {
        currentCommand = 0;
        systemState = IDLE;
        Serial.println(F("Single side paint complete"));
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